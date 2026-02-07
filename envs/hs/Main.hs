{-# LANGUAGE LambdaCase #-}
{-# LANGUAGE NoImplicitPrelude #-}
{-# LANGUAGE OverloadedRecordDot #-}
{-# LANGUAGE OverloadedStrings #-}
{-# LANGUAGE StandaloneDeriving #-}
{-# LANGUAGE TypeApplications #-}

module Main where

import Util
import qualified Initial as I
import qualified Final as F

import qualified Data.ByteString as BS
import qualified Data.Map as Map
import qualified Data.Text as T

main :: IO ()
main = do
  putStrLn $ I.unR $ do
    I.rCType I.CInt
  print $ I.unKC $ do
    I.kindCheck I.CInt
  putStrLn $ F.render $ F.cint
  print $ F.kindCheck $ F.cint
