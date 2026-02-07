{-# LANGUAGE FlexibleInstances #-}

module Final where

import Initial (RenderC(..), CKind(..), KindCheck(..))

class C89TypeSYM a where
  cint :: a
  cchar :: a

--------------------------
------ Interpreters ------
--------------------------

------ Rendering ------

render :: RenderC String -> String
render = unR
instance C89TypeSYM (RenderC String) where
  cint = pure "int"
  cchar = pure "char"

------ Kind Checking ------

kindCheck :: KindCheck CKind -> CKind
kindCheck = unKC
instance C89TypeSYM (KindCheck CKind) where
  cint = pure $ Known 4 4
  cchar = pure $ Known 1 1

