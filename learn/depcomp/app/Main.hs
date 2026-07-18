{-# LANGUAGE OverloadedRecordDot #-}
{-# LANGUAGE GeneralizedNewtypeDeriving #-}

module Main (main) where

import Text.Read (readMaybe)
import System.Environment (getArgs)
import System.IO (hPutStrLn, stderr)
import System.Exit (exitFailure)
import Data.Map (Map)
import Control.Monad.State (State, gets, modify, evalState)
import Data.Maybe (fromMaybe)

import qualified Data.Map as Map

main :: IO ()
main = do
  [fname] <- getArgs
  fileContent <- readFile fname
  ast <- case parseFile fileContent of
    Nothing -> die "syntax error"
    Just it -> pure it
  putStr $ runQbe $ compileProgram ast

---------------------
------ Compile ------
---------------------

compileProgram :: Syntax -> Qbe String
compileProgram ast = compileFunction "main" ast

compileFunction :: String -> Syntax -> Qbe String
-- FIXME we need to gensym instead of hardcoding eg %v0
compileFunction funcName (IntLit n) = do
  v <- gensym "v"
  pure $ unlines
    [ "export function w $"++ funcName ++ "() {"
    , "  @start"
    , "  %" ++ v ++ " =w copy " ++ show n ++ ""
    , "  ret %" ++ v
    , "}"
    ]

newtype Qbe a = Qbe { unQbe :: State QbeSt a }
  deriving (Functor, Applicative, Monad)
data QbeSt = QbeSt
  { varNames :: Map String Int
  }
runQbe :: Qbe a -> a
runQbe (Qbe action) = evalState action st0
  where
  st0 = QbeSt
    { varNames = Map.empty
    }


gensym :: String -> Qbe String
gensym baseName = Qbe $ do
  nextInt <- gets $ fromMaybe 0 . Map.lookup baseName . varNames
  modify $ \st -> st { varNames = Map.insert baseName (nextInt + 1) st.varNames }
  pure $ baseName ++ show nextInt

--------------------
------ Syntax ------
--------------------

{- TODO
I'm already thinking about over-abstracting this.
But to counteract that, how awbout this:
`Syntax` is just an s-expression, and then we have patterns that match the relevant bits of s-expr.
I'd still need a Vault, or an annotation (which I'm really thinking about using the `Syntax f = f (Syntax f)` trick) or something.
Yeah, the annotations are for source location or type annotation or whatever.
-}

data Syntax
  = IntLit Integer
  deriving (Show)

---------------------
------ Parsing ------
---------------------

parseFile :: String -> Maybe Syntax
parseFile inp = IntLit <$> readMaybe inp

-----------------------
------ Utilities ------
-----------------------

die :: String -> IO a
die msg = do
  hPutStrLn stderr msg
  exitFailure
