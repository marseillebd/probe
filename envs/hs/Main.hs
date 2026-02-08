{-# LANGUAGE DataKinds #-}
{-# LANGUAGE FlexibleInstances #-}
{-# LANGUAGE KindSignatures #-}
{-# LANGUAGE LambdaCase #-}
{-# LANGUAGE MultilineStrings #-}
{-# LANGUAGE NoImplicitPrelude #-}
{-# LANGUAGE OverloadedRecordDot #-}
{-# LANGUAGE OverloadedStrings #-}
{-# LANGUAGE StandaloneDeriving #-}

module Main where

import Util

import Control.Monad.State.Strict (State, runState, modify)
import Data.Char (toUpper)
import Data.Kind (Type)

main :: IO ()
main = putStrLn . unR $ file "delme" $ do
  _ <- global "myInt" uintTy (3 + 2)
  func "main" [("other", someType uintTy)] intTy $ do
    ec <- var "exitCode" intTy 0
    pure ec

--------------------
------ Syntax ------
--------------------

type Name = String

data Syntax
  = File
  | Decl
  | Typ Type
  | Stmt
  | Expr Type
  | SomeType -- FIXME this makes me want to use Idris or something

class FileSYM repr where
  file :: Name -> Top repr () -> repr 'File

class DeclSYM repr where
  globalDecl :: Name -> repr ('Typ t) -> repr ('Expr t) -> repr 'Decl
  -- FIXME the params to funcDecl should actually be `exists ts :: Tuple Type. for ts $ \t -> (Name, t)`
  -- and then the body perhaps can take a corresponding `for ts $ \t -> Local t`
  funcDecl :: Name -> [(Name, repr 'SomeType)] -> repr ('Typ r) -> Block repr (Local r) -> repr 'Decl
  -- TODO struct, union
  -- TODO typedef

class TypeSYM repr where
  intTy :: repr ('Typ Int)
  uintTy :: repr ('Typ Word)
  someType :: repr ('Typ t) -> repr 'SomeType

class StmtSYM repr where
  varStmt :: Name -> repr ('Typ t) -> repr ('Expr t) -> repr 'Stmt
  exprStmt :: repr ('Expr t) -> repr 'Stmt

class ExprSYM repr where
  litInt :: Int -> repr ('Expr Int)
  litUint :: Word -> repr ('Expr Word)
  litBool :: Bool -> repr ('Expr Bool)
  add :: (Inty n) => repr ('Expr n) -> repr ('Expr n) -> repr ('Expr n)

class Inty n where
  toExpr :: (ExprSYM repr) => n -> repr ('Expr n)
instance Inty Int where
  toExpr = litInt
instance Inty Word where
  toExpr = litUint

------ Conveniences ------

instance (Inty n, Num n, ExprSYM repr) => Num (repr ('Expr n)) where
  fromInteger i = toExpr $ fromInteger i
  a + b = add a b

-- Where we use 'Block', we could have just used a list of statements.
-- What 'Block' does is make it easy to construct that list,
-- while also allowing us to track the types of the variables it defines.

newtype Local t = Local { unLocal :: String } -- TODO set a scope on those locals

newtype Block repr a = B { unB :: State (repr 'Stmt) a }
runBlock :: (Monoid (repr 'Stmt)) => Block repr a -> (repr 'Stmt, a)
runBlock blk = swap $ runState (unB blk) mempty
execBlock :: (Monoid (repr 'Stmt)) => Block repr a -> repr 'Stmt
execBlock = fst . runBlock

var :: (StmtSYM repr, Monoid (repr 'Stmt))
  => Name -> repr ('Typ t) -> repr ('Expr t)
  -> Block repr (Local t)
var name ty ini = B $ do
  modify (<> varStmt name ty ini)
  pure (Local name)
expr :: (StmtSYM repr, Monoid (repr 'Stmt))
  => repr ('Expr t) -> Block repr ()
expr e = B $ do
  modify (<> exprStmt e)

-- Same reasoning as for 'Block', 'Top' exists instead of a list of declarations.

newtype Global t = Global { unGlobal :: String }

newtype Top repr a = T { unT :: State (repr 'Decl) a }
runTop :: (Monoid (repr 'Decl)) => Top repr a -> (repr 'Decl, a)
runTop unit = swap $ runState (unT unit) mempty
execTop :: (Monoid (repr 'Decl)) => Top repr a -> repr 'Decl
execTop = fst . runTop

global :: (DeclSYM repr, Monoid (repr 'Decl))
  => Name -> repr ('Typ t) -> repr ('Expr t)
  -> Top repr (Global t)
global name ty ini = T $ do
  modify (<> globalDecl name ty ini)
  pure (Global name)

func :: (DeclSYM repr, Monoid (repr 'Decl))
  => Name -> [(Name, repr 'SomeType)] -> repr ('Typ t)
  -> Block repr (Local t)
  -> Top repr ()
func name params retTy body = T $ do
  modify (<> funcDecl name params retTy body)

------ Boilerplate ------

instance Functor (Block repr) where
  fmap f getX = B $ f <$> unB getX
instance Applicative (Block repr) where
  pure x = B $ pure x
  getF <*> getX = B $ do
    f <- unB getF
    x <- unB getX
    pure $ f x
instance Monad (Block repr) where
  getX >>= k = B $ do
    x <- unB getX
    unB $ k x

instance Functor (Top repr) where
  fmap f getX = T $ f <$> unT getX
instance Applicative (Top repr) where
  pure x = T $ pure x
  getF <*> getX = T $ do
    f <- unT getF
    x <- unT getX
    pure $ f x
instance Monad (Top repr) where
  getX >>= k = T $ do
    x <- unT getX
    unT $ k x

--------------------
------ Render ------
--------------------

newtype Render (syntax :: Syntax) = R { unR :: String }

instance FileSYM Render where
  file name decls = R $ unlines
    [ "#ifndef " ++ (toUpper <$> name) ++ "_H"
    , unR $ execTop decls
    , "#endif"
    ]

instance Semigroup (Render 'Decl) where
  (R "") <> b = b
  a <> (R "") = a
  a <> b = R $ unR a <> "\n" <> unR b
instance Monoid (Render 'Decl) where
  mempty = R ""

instance DeclSYM Render where
  globalDecl name ty ini = R $ concat
    [ unR ty, " ", name, " = ", unR ini, ";" ]
  funcDecl name params ret body = R $ concat
    [ unR ret, " ", name, "("
    , intercalate ", " $ flip map params $ \(pName, pType) ->
      concat [ unR pType, " ", pName ]
    , ")\n{\n"
    , let (bodyR, retX) = runBlock body
      in unlines
        [ unR bodyR
        , "return " <> retX.unLocal <> ";"
        ]
    , "\n}"
    ]

instance Semigroup (Render 'Stmt) where
  (R "") <> b = b
  a <> (R "") = a
  a <> b = R $ unR a <> "\n" <> unR b
instance Monoid (Render 'Stmt) where
  mempty = R ""

instance StmtSYM Render where
  varStmt name ty ini = R $ concat
    [ unR ty, " ", name , " = ", unR ini , ";" ]
  exprStmt e = R $ concat
    [ unR e, ";" ]

instance TypeSYM Render where
  intTy = R $ "int"
  uintTy = R $ "unsigned int"
  someType = R . unR

instance ExprSYM Render where
  litInt i = R $ show i
  litUint i = R $ show i
  litBool b = R $ if b then "true" else "false" -- TODO import stdbool
  add a b = R $ "(" ++ unR a ++ ") + (" ++ unR b ++ ")"

