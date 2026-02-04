module Util
  ( module X
  -- Hardened &/ Generalized Prelude
  , head, unsafeHead
  -- * Data Structures
  , ByteString, Text
  , Map
  -- * Functions, Functors, &c
  , on, (&), (<&>)
  -- * Control Flow
  , maybeM_, forM, forM_
  -- * Shell-type Shii
  , mkdir, mkdir_p
  , rmdir, rmdir_r, rmdir_rf
  , (</>), (<.>), (-<.>)
  ) where

import Prelude as X hiding (head)
import qualified Prelude

import System.Directory

import Control.Monad (void, forM, forM_)
import Data.ByteString (ByteString)
import Data.Function ((&), on)
import Data.Functor ((<&>))
import Data.Map (Map)
import Data.Text (Text)
import System.FilePath ((</>), (<.>), (-<.>))

head :: [a] -> Maybe a
head [] = Nothing
head (x:_) = Just x

unsafeHead :: [a] -> a
unsafeHead = Prelude.head

maybeM_ :: (Applicative m) => Maybe a -> (a -> m x) -> m ()
maybeM_ Nothing _ = pure ()
maybeM_ (Just x) action = void $ action x

mkdir, mkdir_p :: FilePath -> IO ()
mkdir = createDirectory
mkdir_p = createDirectoryIfMissing True

rmdir, rmdir_r, rmdir_rf :: FilePath -> IO ()
rmdir = removeDirectory
rmdir_r = removeDirectoryRecursive
rmdir_rf = removePathForcibly
