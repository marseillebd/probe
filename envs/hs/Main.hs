{-# LANGUAGE LambdaCase #-}
{-# LANGUAGE NoImplicitPrelude #-}
{-# LANGUAGE OverloadedRecordDot #-}
{-# LANGUAGE OverloadedStrings #-}
{-# LANGUAGE RecordWildCards #-}
{-# LANGUAGE StandaloneDeriving #-}

module Main where

import Util

import Control.Monad.ST (ST, runST)
import Data.List (intercalate)
import Data.STRef (STRef, newSTRef, readSTRef, writeSTRef)

import qualified Data.ByteString as BS
import qualified Data.Map as Map
import qualified Data.Text as T

main :: IO ()
main = do
  putStrLn "Hello, Haskell!"
  let tree =
        Conclusion (Elaborate (App (Const "negInt") (Const "1"))
                      (Const "Int")
                      (App (Const "negInt") (Const "1")))
          ("App",
          [ Conclusion (Elaborate (Const "negInt")
                        (App (App (Const "Fn") (Const "Int")) (Const "Int"))
                        (Const "negInt"))
            ("Delta", [])
          , Conclusion (Elaborate (Const "1")
                        (Const "Int")
                        (Const "1"))
            ("Delta", [])
          ])
  putStrLn $ showL tree

-- TODO: next step
-- I want to manually build a prooftree and see what it looks like.
-- I'll need a pretty-printer/2d text renderer.
-- I'll need to display terms and contexts, preferrably trimming deep nesting.

---------------------------
------ The Prooftree ------
---------------------------

data Prooftree
  = Conclusion Judgement (Rule, [Prooftree]) -- a line that is justified by a rule referencing preceding lines
  | Subproof Context Prooftree -- a proof relative to a context extension

type Rule = String

----------------------------
------ The Judgements ------
----------------------------

-- In a fitch-style proof, we need to fill out the slots CTX and WFF, as in the following example.
-- The numbers identifying each row on the left and the reasoning chain referencing rules and line numbers on the right
--   will both be given in the structure of the proof tree, rather than in the "judgements"
-- ```
-- | CTX1
-- |------
-- | WFF1
-- | | CTX2
-- | |------
-- | | WFF2
-- | | WFF3
-- | WFF4
-- WFF5
-- ```

-- In this experimental case, we only really have one kind of judgement, and it is built out of terms,
--   and only one kind of context element, also built of terms.

type Judgement = Elaborate
data Elaborate = Elaborate
  { srcTerm :: Term
  , expectType :: Term
  , outTerm :: Term
  }

type Context = [CtxItem]
data CtxItem
  = VarType (VarName, Term)

type VarName = String

data Term
  = Var VarName
  | Fun (VarName, Term) Term
  | App Term Term
  | Const String
  | MetaTerm Metavar

-- While building the prooftree, we will encounter places where we need to fill in an appropriate term.
-- This will be done by unification, but perhaps only after the prooftree is built.
-- In the meantime, we need to put these placeholders into the term formulae.
data Metavar = Mv
  { name :: String
  , ref :: IO [ThingsToUnifyAndWhy]
  }
type ThingsToUnifyAndWhy = () -- TODO

-----------------------
------ Rendering ------
-----------------------

-- FIXME I could stand to show terms a little deeper into the prooftree,
-- and probably use some special typography for summarized terms.
--FIXME: I also need to work on `seqL` to render subtrees.

class ShowLogic a where
  showL :: a -> String

instance ShowLogic Metavar where
  showL = name

instance ShowLogic Term where
  showL (Var x) = x
  showL (Fun (x, ty) body) = concat ["λ", x, " :: t_", x, ". e"]
  showL (App f e) = "f e"
  showL (Const c) = c
  showL (MetaTerm alpha) = showL alpha

instance ShowLogic CtxItem where
  showL (VarType (x, ty)) = concat [x, " :: ", "t_", x]

instance ShowLogic Elaborate where
  showL Elaborate{..} = concat
    [ showL srcTerm, " :: "
    , showL expectType, " ~~> "
    , showL outTerm
    ]

instance ShowLogic Prooftree where
  showL tree =
    let lineCols = runST $ do
          (prelines, _) <- seqL 1 tree
          numL prelines
          forM prelines unpreline
        max1 = maximum $ map (\(x, _, _) -> length x) lineCols
        max2 = maximum $ map (\(_, x, _) -> length x) lineCols
        theLines = flip map lineCols $ \(no, j, by) -> concat
          [ take max1 $ no ++ repeat ' '
          , take max2 $ j ++ repeat ' '
          , "    " , by
          ]
     in unlines theLines

data Preline s = Preline
  { lineNo :: STRef s [Int]
  , lineDepth :: Int
  , lineJudge :: String
  , lineRule :: String
  , lineRefs :: [STRef s [Int]]
  }

seqL :: Int -> Prooftree -> ST s ([Preline s], STRef s [Int])
seqL depth (Conclusion j (r, deps)) = do
  stuff <- forM deps $ \subtree -> do
    seqL depth subtree
  let lines = concat $ fst <$> stuff
  let refs = snd <$> stuff
  myNo <- newSTRef (error "lineno not set")
  let myLine = Preline
        { lineNo = myNo
        , lineDepth = depth
        , lineJudge = showL j
        , lineRule = r
        , lineRefs = refs
        }
  pure (lines ++ [myLine], myNo)

numL :: [Preline s] -> ST s ()
numL lines = do
  ctrRef <- newSTRef [0]
  forM_ lines $ \Preline{..} -> do
    ctr <- readSTRef ctrRef
    let depth = length ctr
        depth' = lineDepth
        ctr' = case depth' - depth of
          n | n == 0 -> inc ctr
            | n > 0 -> into n ctr
            | n < 0 -> outof n ctr
    writeSTRef lineNo ctr'
    writeSTRef ctrRef ctr'

unpreline :: Preline s -> ST s (String, String, String)
unpreline Preline{..} = do
  noStr <- showCtr lineNo
  let vlines = concat $ replicate lineDepth " |"
  let j = if null lineJudge then "-------------"
          else " " ++ lineJudge
  refNos <- forM lineRefs $ showCtr
  let by = lineRule ++ " " ++ intercalate ", " refNos
  pure (noStr, vlines ++ j, by)
  where
  showCtr :: STRef s [Int] -> ST s String
  showCtr ref = do
    ns <- readSTRef ref
    pure $ intercalate "." $ show <$> ns

inc :: [Int] -> [Int]
into, outof :: Int -> [Int] -> [Int]
inc (x:xs) = (x+1:xs)
into n xs = replicate n 1 ++ inc xs
outof n xs = inc $ drop n xs
