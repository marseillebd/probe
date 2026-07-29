{-# LANGUAGE LambdaCase #-}
{-# LANGUAGE MultilineStrings #-}
{-# LANGUAGE NoImplicitPrelude #-}
{-# LANGUAGE NumericUnderscores #-}
{-# LANGUAGE OverloadedRecordDot #-}
{-# LANGUAGE OverloadedStrings #-}
{-# LANGUAGE StandaloneDeriving #-}

module Main where

import Util

import Control.Concurrent
import System.CPUTime

main :: IO ()
main = do
  putStrLn "Hello, Haskell!"
  let nThreads = 4
  -- set up a channel for runners to signal completion
  runnerReady <- newChan :: IO (Chan (Chan (Maybe (Int, String))))
  -- create runner threads and channels to send them each jobs
  sendJobs <- forM [(1::Int)..nThreads] $ const newChan
  runners <- forM sendJobs $ \recvJob -> do
    let run job = runner job >> writeChan runnerReady recvJob >> loop
        loop = readChan recvJob >>= maybe pass run
    writeChan runnerReady recvJob -- establish the runner as initially ready
    forkIO loop
  forM_ runners $ \tid -> putStrLn $
    "thread " ++ show tid ++ " started"
  -- send jobs to runners as they become ready
  mapM_ (\job -> readChan runnerReady >>= flip writeChan (Just job))
    [ (1, "A")
    , (3, "B")
    , (5, "C")
    , (2, "D")
    , (1, "E")
    ]
  -- wait for all runners signal completion before exiting
  forM_ [1..nThreads] $ \_ -> do
    void $ readChan runnerReady
    putStrLn $ "thread ??? done"



runner :: (Int, String) -> IO ()
runner (secs, msg) = do
  tid <- myThreadId
  t0 <- getCPUTime
  putStrLn $ "thread " ++ show tid ++ " sleeping for " ++ show secs ++ " seconds"
  threadDelay $ secs * 1_000_000 -- seconds to microseconds
  t1 <- getCPUTime
  let dt = (t1 - t0) `div` 1_000_000 -- picoseconds to microseconds
  putStrLn $ "thread " ++ show tid ++ " slept for " ++ show dt ++ "ms"
  putStrLn $ "thread " ++ show tid ++ " says: " ++ msg
