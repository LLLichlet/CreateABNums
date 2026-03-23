import Data.Array
import Data.Bits
import Data.Maybe
import qualified Data.IntMap as M

type Mask = Int
type Cost = Int
type CostVec = Array Int Cost
type CostMap = M.IntMap CostVec
type ParentMap = M.IntMap (Mask, Operation)

data Operator = Add | Sub | Mul | Div deriving (Show, Eq, Enum)
data Operation = Op
  { opX   :: Int
  , opY   :: Int
  , opRes :: Int
  , opSym :: Operator
  } deriving (Show)

genOps :: Int -> Int -> [Operation]
genOps x y = 
  [ Op x y res op
  | (op, fn) <- zip [Add,Sub,Mul,Div] [ (+), (-), (*), div' ]
  , let res = fn x y
  , res >= 0 && res <= 9
  , res /= x && res /= y
  , if op `elem` [Add,Mul] then x <= y else True
  ]
  where
    div' a b = if b == 0 || a `mod` b /= 0 then -1 else a `div` b

initMask :: Mask
initMask = (1 `shiftL` 3) .|. (1 `shiftL` 8)

goalMask :: Mask
goalMask = (1 `shiftL` 10) - 1

initCostVec :: CostVec
initCostVec = array (0,9) $ 
  [(i, if i==3 || i==8 then 1 else maxBound) | i<-[0..9]]

step :: (CostMap, ParentMap) -> (CostMap, ParentMap)
step (cm, pm) = foldl update (cm, pm) allTries
  where
    allTries = 
      [ (s, op)
      | (s, vec) <- M.toList cm
      , a <- [0..9], testBit s a
      , b <- [0..9], testBit s b
      , op <- genOps a b
      ]

    update (cmAcc, pmAcc) (s, op) =
      let vec = cmAcc M.! s
          x = opX op; y = opY op; r = opRes op
          newCost = vec!x + vec!y + 1
          s' = setBit s r
          vec' = cmAcc M.!? s'
      in case vec' of
           Nothing -> 
             let newVec = vec // [(r, newCost)]
             in ( M.insert s' newVec cmAcc
                , M.insert s' (s, op) pmAcc )
           Just oldVec | oldVec!r > newCost ->
             let newVec = vec // [(r, newCost)]
             in ( M.insert s' newVec cmAcc
                , M.insert s' (s, op) pmAcc )
           _ -> (cmAcc, pmAcc)

solve :: (CostMap, ParentMap)
solve = until converged (applyStep) (initialMap, M.empty)
  where
    initialMap = M.singleton initMask initCostVec
    applyStep = step
    converged (cm, _) = case M.lookup goalMask cm of
      Just vec -> all (< maxBound) (elems vec)
      Nothing  -> False

backtrack :: ParentMap -> Mask -> [Operation]
backtrack pm m = reverse $ go m
  where
    go m = case M.lookup m pm of
      Nothing -> []
      Just (parent, op) -> op : go parent

printResult :: (CostMap, ParentMap) -> IO ()
printResult (cm, pm) = do
  let Just finalVec = M.lookup goalMask cm
  putStrLn "cost of nums:"
  print $ elems finalVec
  putStrLn $ "sum cost = " ++ show (sum (elems finalVec))
  let ops = backtrack pm goalMask
  mapM_ (\(i, op) -> 
           putStrLn $ "operation #" ++ show i ++ ": " ++ show (opX op) ++ " " ++ [symToChar (opSym op)] ++ " " ++ show (opY op) ++ " = " ++ show (opRes op))
        (zip [1 :: Int ..] ops)
  where
    symToChar Add = '+'; symToChar Sub = '-'; symToChar Mul = '*'; symToChar Div = '/'

main :: IO ()
main = printResult solve
