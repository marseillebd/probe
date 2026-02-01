{-# LANGUAGE LambdaCase #-}
{-# LANGUAGE NoImplicitPrelude #-}
{-# LANGUAGE OverloadedRecordDot #-}
{-# LANGUAGE OverloadedStrings #-}
{-# LANGUAGE StandaloneDeriving #-}

module Main where

import Util

import Data.List (isPrefixOf, groupBy)
import Data.Function ((&), on)

import qualified Data.ByteString as BS
import qualified Data.Map as Map
import qualified Data.Text as T

deriving instance Show TypedSrc -- DEBUG
deriving instance Show SrcType -- DEBUG
deriving instance Show SrcPair -- DEBUG
deriving instance Show SrcFile -- DEBUG

main :: IO ()
main = do
  input <- getContents
  let results = input
-- Hocco understands lines of "plain text".
-- I have a rant(s) about plain text, but for this quick-and-dirty implementation, Haskell's stock `lines` function is probably good enough.
        & lines
-- We check the first line (and only the first) to see if it's a shebang.
-- For every other line, we defer line parsing to `parseTySrc`
        & \case
          x : xs | "#!" `isPrefixOf` x
            -> TySrc Shebang x : map parseTySrc xs
          xs -> map parseTySrc xs
-- Group lines by type and then pair documentation with code.
        & groupSrc
        & pairSrc
  print results.shebang
  mapM_ print results.rows

-- # Parsing

-- ## Input Classification

-- The objective here is to separate out the (literate) documenation from the code.
-- To do this, we tag each line with a type and then group types together.
data TypedSrc = TySrc
  { ty :: SrcType
  , txt :: String
  }

data SrcType
  = Docs
  | Code
  | Shebang -- TODO what if I deleted this type and instead just passed a tuple in the main pipeline while mapping with `second`?
  -- TODO there's the issue of whether blank lines should be code always or included with whatever the prior type was
  deriving (Eq)

-- To introduce `SrcType`, we simply feed one line at a time and look at the first few bytes.
parseTySrc :: String -> TypedSrc
parseTySrc str
-- If the line begins in a line-comment followed by space (not tab), then it's part of the documentation.
-- Note that this must be at the start of the line, **not** after some spaces.
  | indicator `isPrefixOf` str = TySrc Docs $ (drop $ length indicator) str
-- Anything else is treated as code, including blank lines.
  | otherwise = TySrc Code str
  -- TODO make this indicator configurable
  where
  indicator = "-- "

-- ## Input Grouping

-- Although individual lines are classified, they relate to their neighbors.
-- Adjacent lines of the same type together form a _block_.
-- We get to reuse the `TySrc` for blocks, since the type is agnostic as to what text it contains.
--
-- Note: the use of `head`, though a partial function, is safe here,
-- because `groupBy` always produces a list of non-empty lists.
groupSrc :: [TypedSrc] -> [TypedSrc]
groupSrc srclines = srclines
  & groupBy ((==) `on` (.ty))
  & map (\ys -> TySrc (unsafeHead ys).ty (unlines $ map (.txt) ys))

-- Recall that the output format is intended to place documentation side-by-side with its corresponding code.
-- Here we pair up a documentation block with the following code block.
data SrcPair = SrcPair
  { docs :: String
  , code :: String
  }

-- However, we also need to remember the shebang line.
-- I thought about using just `(Maybe String, [SrcPair])`, but I distrust structural types.
-- Thus, I define another record to help track the semantics.
data SrcFile = SrcFile
  { shebang :: Maybe String
  , rows :: [SrcPair]
  }

-- We again treat the first list entry specially for shebangs,
-- but mostly delegate to `loop`.
pairSrc :: [TypedSrc] -> SrcFile
pairSrc = \case
  l1 : body | l1.ty == Shebang -> SrcFile (Just l1.txt) (loop body)
  body -> SrcFile Nothing (loop body)
  where
-- Most of the time, we should get documentation followed by code.
  loop :: [TypedSrc] -> [SrcPair]
  loop (a : b : rest)
    | a.ty == Docs, b.ty == Code
      = SrcPair a.txt b.txt : loop rest
-- We might get some code that has no associated documentation, though. (At the start of the file.)
    | a.ty == Code
      = SrcPair "" a.txt : loop (b : rest)
-- Likewise, we might have documentation with no following code. (At the end of a file, so we terminate.)
  loop [a]
    | a.ty == Docs
      = SrcPair a.txt "" : []
-- The loop terminates without fanfare.
  loop [] = []
-- We should never run into a shebang type.
  loop (a : _) | a.ty == Shebang = errorWithoutStackTrace "internal error - unexpected shebang block"
