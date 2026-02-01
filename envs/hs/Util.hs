module Util
  ( module X
  , head, unsafeHead
  ) where

import Prelude as X hiding (head)
import qualified Prelude

import Data.ByteString (ByteString)
import Data.Map (Map)
import Data.Text (Text)

head :: [a] -> Maybe a
head [] = Nothing
head (x:_) = Just x

unsafeHead :: [a] -> a
unsafeHead = Prelude.head
