{-# LANGUAGE DeriveFunctor #-}

module Initial where

data CType
  = CInt
  | CChar

type Name = String -- TODO

data File = File
  { dataDecls :: [DataDecl]
  }

data DataDecl = DataDecl
  { ddeclType :: CType
  , ddeclName :: Name
}

--------------------------
------ Interpreters ------
--------------------------

------ Rendering ------

newtype RenderC a = R { unR :: a }
  deriving(Functor)
instance Applicative RenderC where
  pure x = R $ x
  R getF <*> R getX = R $ getF getX
instance Monad RenderC where
  getX >>= k = k $ unR getX

rCType :: CType -> RenderC String
rCType CInt = pure "int"
rCType CChar = pure "char"

------ Kind Checking ------

data CKind
  = Opaque
  | Known
    { sizeof :: Int
    , alignof :: Int
    }
  deriving (Show)

newtype KindCheck a = KC { unKC :: a }
  deriving(Functor)
instance Applicative KindCheck where
  pure x = KC $ x
  KC getF <*> KC getX = KC $ getF getX
instance Monad KindCheck where
  getX >>= k = k $ unKC getX

kindCheck :: CType -> KindCheck CKind
kindCheck CInt = pure $ Known 4 4
kindCheck CChar = pure $ Known 1 1
