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

import Data.Char (toUpper)
import Data.Kind (Type)

main :: IO ()
main = putStrLn . unR $ file "delme"
  [ varDef "myInt" uintTy (3 + 2)
  ]

--------------------
------ Syntax ------
--------------------

type Name = String

data Syntax
  = File
  | Decl
  | Typ Type
  | Expr Type

class FileSYM a where
  file :: Name -> [a Decl] -> a File

class DeclSYM a where
  varDecl :: Name -> a Decl
  varDef :: Name -> a (Typ t) -> a (Expr t) -> a Decl

class TypeSYM a where
  intTy :: a (Typ Int)
  uintTy :: a (Typ Word)

class ExprSYM repr where
  litInt :: Int -> repr (Expr Int)
  litUint :: Word -> repr (Expr Word)
  litBool :: Bool -> repr (Expr Bool)
  add :: (Inty n) => repr (Expr n) -> repr (Expr n) -> repr (Expr n)

class Inty n where
  toExpr :: (ExprSYM repr) => n -> repr (Expr n)
instance Inty Int where
  toExpr = litInt
instance Inty Word where
  toExpr = litUint

------ Conveniences ------

instance (Inty n, Num n, ExprSYM repr) => Num (repr (Expr n)) where
  fromInteger i = toExpr $ fromInteger i
  a + b = add a b

--------------------
------ Render ------
--------------------

newtype Render (syntax :: Syntax) = R { unR :: String }

instance FileSYM Render where
  file name decls = R $ unlines
    [ "#ifndef " ++ (toUpper <$> name) ++ "_H"
    , unlines $ unR <$> decls
    , "#endif"
    ]

instance DeclSYM Render where
  varDecl name = R $ "int " ++ name ++ ";"
  varDef name ty ini = R $ unR ty ++ " " ++ name ++ " = " ++ unR ini ++ ";"

instance TypeSYM Render where
  intTy = R $ "int"
  uintTy = R $ "unsigned int"

instance ExprSYM Render where
  litInt i = R $ show i
  litUint i = R $ show i
  litBool b = R $ if b then "true" else "false" -- TODO import stdbool
  add a b = R $ "(" ++ unR a ++ ") + (" ++ unR b ++ ")"
