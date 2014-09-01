module Main where

import Prelude hiding (Left, Right)
import Test.QuickCheck.Gen
import Control.Applicative
import Control.Monad.State

data Op = Plus | Minus | Mult | And | Or
        deriving (Show)

data Dir = Left | Right
         deriving (Show)
                  
type N32 = Int -- but only 1..30 in generator

data Expr = BinOp Op Expr Expr
          | ShiftOp Dir Expr N32
          | Num Int
          | T
          deriving (Show)

type GenBuilder = StateT Int Gen

maxDepth :: Int
maxDepth = 10

oneof' :: [GenBuilder a] -> GenBuilder a
oneof' = join . lift . elements

nest :: GenBuilder Expr -> GenBuilder Expr
nest = (modify (+1) >>)

randExpr :: Bool -> GenBuilder Expr
randExpr canBeNum = do
  depth <- get
  if depth < maxDepth
    then oneof' ((if canBeNum then [Num <$> num] else [])
                 ++ [shiftOp, shiftOp, binOp, binOp, binOp, return T])
    else Num <$> num
  where binOp = BinOp <$> lift (elements [Plus, Minus, Mult, And, Or])
                <*> nest (randExpr canBeNum) <*> nest (randExpr canBeNum)
        shiftOp = ShiftOp <$> lift (elements [Left, Right])
                  <*> nest (randExpr False) <*> num
        num = lift (choose (1, 6))


formatExpr :: Expr -> String
formatExpr e = case e of
  BinOp op a b -> "(" ++ formatExpr a ++ formatOp op ++ formatExpr b ++ ")"
  ShiftOp dir e n -> "(" ++ formatExpr e ++ formatDir dir ++ show n ++ ")"
  Num n -> show n
  T -> "t"

formatOp :: Op -> String
formatOp o = case o of
  Plus -> "+"
  Minus -> "-"
  Mult -> "*"
  And -> "&"
  Or -> "|"

formatDir :: Dir -> String
formatDir d = case d of
  Left -> "<<"
  Right -> ">>"

main = do
  s <- liftM (formatExpr . head) $ sample' (evalStateT (randExpr True) 0)
  putStrLn s
