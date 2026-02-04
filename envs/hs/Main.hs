{-# LANGUAGE LambdaCase #-}
{-# LANGUAGE MultilineStrings #-}
{-# LANGUAGE NoImplicitPrelude #-}
{-# LANGUAGE OverloadedRecordDot #-}
{-# LANGUAGE OverloadedStrings #-}
{-# LANGUAGE StandaloneDeriving #-}

module Main where

import Util

import Data.List (isPrefixOf, groupBy, intercalate)
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
            -> TySrc 1 Shebang x : zipWith parseTySrc [2..] xs
          xs -> zipWith parseTySrc [1..] xs
-- Group lines by type and then pair documentation with code.
        & groupSrc
        & pairSrc
  let outdir = "qd"
  rmdir_rf outdir
  mkdir_p outdir
  -- write separated file parts to individual files
  maybeM_ results.shebang $ \sb -> do
    writeFile (outdir </> "000001.shebang.qd") sb
  forM_ results.rows $ \pair -> do
    let lineStr = show pair.codeStartLine
        seqStr = drop (length lineStr) "000000" <> lineStr -- six digits actually allows line numbers to be sorted write up to seven 9s
    writeFile (outdir </> seqStr <.> "docs.qd") pair.docs
    writeFile (outdir </> seqStr <.> "code.qd") pair.code
  -- Merge into a single, basic html file
  withHtml $ do
    case (results.shebang, head results.rows) of
      (sbish, Just hd) -> do
        withDocHtml hd.docs
        withCodeHtml sbish hd.codeStartLine hd.code
      (Just sb, Nothing) -> do
        withDocHtml ""
        withCodeHtml (Just sb) 2 ""
      (Nothing, Nothing) -> pure ()
    forM_ (drop 1 results.rows) $ \pair -> do
      withDocHtml pair.docs
      withCodeHtml Nothing pair.codeStartLine pair.code

withHtml :: IO () -> IO ()
withHtml action = do
  putStrLn startHtml
  action
  putStrLn endHtml
  where

  startHtml = """
  <!DOCTYPE html>
  <html>
    <head>
      <meta charset=utf-8>
      <style>
  """ ++ css ++ """
      </style>
      <script>
  """ ++ js ++ """
      </script>
    </head>
    <body>
      <div id=literate-content>
  """

  endHtml :: String
  endHtml = """
      </div>
    </body>
  </html>
  """

  css = """
  #literate-content {
    display: grid;
    grid-template-columns: 1fr 1fr;
  }

  #literate-content { counter-reset: line; }
  #literate-content > div > code > pre:before {
    counter-increment: line;
    content: counter(line);
  }

  #literate-content details {
    margin-left: 1em;
  }
  """

  js = ""

withDocHtml :: String -> IO ()
withDocHtml inner = do
  putStrLn "<div><pre>"
  putStrLn $ escapeHtml inner
  putStrLn "</pre></div>"

withCodeHtml :: Maybe String -> Int -> String -> IO ()
withCodeHtml sbish lno inner = do
  maybeM_ sbish $ \sb -> do
    putStrLn "<div><code>"
    putStr $ "<pre>" ++ escapeHtml sb ++ "</pre>"
    putStrLn "</div></code>"
  putStrLn "<div><code>"
  putStr $ "<span style=\"counter-set: line " ++ show (lno - 1) ++";\"></span>"
  forM_ (lines inner) $ \l -> do
    putStr $ "<pre>" ++ escapeHtml l ++ "</pre>"
  putStrLn "</div></code>"

-- # Parsing

-- ## Input Classification

-- The objective here is to separate out the (literate) documenation from the code.
-- To do this, we tag each line with a type and then group types together.
data TypedSrc = TySrc
  { startLine :: Int
  , ty :: SrcType
  , txt :: String
  }

data SrcType
  = Docs
  | Code
  | Shebang -- TODO what if I deleted this type and instead just passed a tuple in the main pipeline while mapping with `second`?
  -- TODO there's the issue of whether blank lines should be code always or included with whatever the prior type was
  deriving (Eq)

-- To introduce `SrcType`, we simply feed one line at a time and look at the first few bytes.
parseTySrc :: Int -> String -> TypedSrc
parseTySrc i str
-- If the line begins in a line-comment followed by space (not tab), then it's part of the documentation.
-- Note that this must be at the start of the line, **not** after some spaces.
  | (indicator ++ " ") `isPrefixOf` str = TySrc i Docs $ (drop $ 1 + length indicator) str
  | indicator == str = TySrc i Docs ""
-- Anything else is treated as code, including blank lines.
  | otherwise = TySrc i Code str
  -- TODO make this indicator configurable
  where
  indicator = "--"

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
  & map (\ys ->
    let hd = unsafeHead ys
    in  TySrc hd.startLine hd.ty (unlines $ map (.txt) ys))

-- Recall that the output format is intended to place documentation side-by-side with its corresponding code.
-- Here we pair up a documentation block with the following code block.
data SrcPair = SrcPair
  { docs :: String
  , codeStartLine :: Int
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
      = SrcPair a.txt b.startLine b.txt : loop rest
-- We might get some code that has no associated documentation, though. (At the start of the file.)
  loop (a : rest)
    | a.ty == Code
      = SrcPair "" a.startLine a.txt : loop rest
-- Likewise, we might have documentation with no following code. (At the end of a file, so we terminate.)
  loop [a]
    | a.ty == Docs
      = SrcPair a.txt a.startLine "" : [] -- FIXME this is the start line of the docs, but I think I could figure out the line past the end of the file
-- The loop terminates without fanfare.
  loop [] = []
-- We should never run into a shebang type, or any other combo of focs and code.
  loop (a : _) | a.ty == Shebang = errorWithoutStackTrace "internal error - pairing found unexpected shebang block"
  loop (a : b : _) = errorWithoutStackTrace $ "internal error - pairing found " <> show a.ty <> " " <> show b.ty
  loop (a : _) = errorWithoutStackTrace $ "internal error - pairing found " <> show a.ty <> " <none>"

-- Function to escape HTML special characters
escapeHtml :: String -> String
escapeHtml = concatMap escapeChar
  where
  escapeChar '<' = "&lt;"
  escapeChar '>' = "&gt;"
  escapeChar '&' = "&amp;"
  escapeChar '"' = "&quot;"
  escapeChar '\'' = "&apos;"
  escapeChar c = [c]  -- Return the character unchanged
