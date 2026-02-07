{-# LANGUAGE DataKinds #-}
{-# LANGUAGE DeriveFunctor #-}
{-# LANGUAGE GADTs #-}
{-# LANGUAGE KindSignatures #-}

module Initial where

data CStd = C89 | C99
class InC99 (std :: CStd) where
instance InC99 C89 where
instance InC99 C99 where

data CType std where
  CInt :: CType C89
  CChar :: CType C89
  CInt8 :: CType C99

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

rCType :: (InC99 std) => CType std -> RenderC String
rCType CInt = pure "int"
rCType CChar = pure "char"
rCType CInt8 = pure "int8_t"

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

kindCheck :: CType C89 -> KindCheck CKind
kindCheck CInt = pure $ Known 4 4
kindCheck CChar = pure $ Known 1 1
