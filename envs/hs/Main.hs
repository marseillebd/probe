{-# LANGUAGE LambdaCase #-}
{-# LANGUAGE MultilineStrings #-}
{-# LANGUAGE NoImplicitPrelude #-}
{-# LANGUAGE OverloadedRecordDot #-}
{-# LANGUAGE OverloadedStrings #-}
{-# LANGUAGE StandaloneDeriving #-}

module Main where

import Util
import Control.Monad (guard, forM_)
import Data.List (sortOn)

import qualified Data.ByteString as BS
import qualified Data.Map as Map
import qualified Data.Text as T

zone :: Int -> [Int]
zone 0 = [0, 0, 0]
zone 1 = [1, 0, 0]
zone 2 = [0, 1, 0]
zone 3 = [0, 0, 1]

zones :: Int -> [[Int]]
zones n = [zone i | i <- [0..n]]

allCodes :: [[Int]]
allCodes = do
  w <- map (take 1) $ zones 1
  z <- reverse $ zones 3
  y <- reverse $ zones 3
  x <- reverse $ zones 3
  let card = w ++ x ++ y ++ z
  guard $ sum card > 0
  pure card

main :: IO ()
main = forM_ (sortOn sum allCodes) $ \code -> do
  code
    & map (\i -> if i == 0 then ' ' else 'X')
    & putStrLn
