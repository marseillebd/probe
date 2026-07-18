module Main (main) where

import Text.Read (readMaybe)
import System.Environment (getArgs)
import System.IO (hPutStrLn, stderr)
import System.Exit (exitFailure)

main :: IO ()
main = do
  [fname] <- getArgs
  fileContent <- readFile fname
  ast <- case parseFile fileContent of
    Nothing -> die "syntax error"
    Just it -> pure it
  putStr $ compileProgram ast

---------------------
------ Compile ------
---------------------

compileProgram :: Syntax -> String
compileProgram ast = compileFunction "main" ast

compileFunction :: String -> Syntax -> String
compileFunction funcName (IntLit n) =
  "export function w $"++ funcName ++ "() {\n\
  \  @start\n\
  \  %v0 =w copy " ++ show n ++ "\n\
  \  ret %v0\n\
  \}\n\
  \"

--------------------
------ Syntax ------
--------------------

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
