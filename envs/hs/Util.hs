module Util
  ( module X
  -- Hardened &/ Generalized Prelude
  , head, unsafeHead
  -- * Control Flow
  , maybeM_, forM, forM_
  -- * Shell-type Shii
  , mkdir, mkdir_p
  , (</>), (<.>), (-<.>)
  ) where

import Prelude as X hiding (head)
import qualified Prelude

import System.Directory

import Control.Monad (void, forM, forM_)
import Data.ByteString (ByteString)
import Data.Map (Map)
import Data.Text (Text)
import System.FilePath ((</>), (<.>), (-<.>))

head :: [a] -> Maybe a
head [] = Nothing
head (x:_) = Just x

unsafeHead = Prelude.head

maybeM_ :: (Applicative m) => Maybe a -> (a -> m x) -> m ()
maybeM_ Nothing _ = pure ()
maybeM_ (Just x) action = void $ action x

mkdir = createDirectory
mkdir_p = createDirectoryIfMissing True
