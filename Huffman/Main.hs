-- Christian Bach Møllnitz
module Main where

import Data.List
import Data.Maybe

-- Primary demonstration function, handles all data in the REPL locally.
main :: IO ()
main = do 
    putStrLn("Huffman Tree Direct parse! - enter your string:")
    string <- getLine
    putStrLn("Parsing to Huffman: " ++ string)
    let asso = assosiation string
        leafs = map (\x -> (Leaf x)) asso
        tree = construct_htree leafs
        encodes = encodeStrings tree
    putStrLn("Your Huffman tree: " ++ show tree)
    putStrLn("Your encoding paths: " ++ show encodes)
    putStrLn("You can now encode and decode words with your huffman tree, type 'exit' if you want to exit the loop, 'letters' if you want a list of available letters")
    askTree tree encodes

-- Save, writes a Huffman tree to a file, that can then be loaded again.
save :: IO ()
save = do
    putStrLn("Huffman Tree Saver! - enter your string:")
    string <- getLine
    putStrLn("Enter your file name:")
    filename <- getLine
    putStrLn("Parsing to Huffman: " ++ string)
    let asso = assosiation string
        leafs = map (\x -> (Leaf x)) asso
        tree = construct_htree leafs
    putStrLn("Saving Huffman tree... " ++ show tree)
    writeFile filename (show tree)
    putStrLn("Save complete, you can now do 'load'  and then enter " ++ filename ++ " as your filename!")

-- Loads a Huffman tree, (Written first by save!) - and then lets the user encode and decode words recursively
load :: IO ()
load = do 
    putStrLn("Huffman Tree Loader!")
    putStrLn("Enter your file name:")
    filename <- getLine
    putStrLn("Loading from: " ++ filename)
    test <- readFile filename
    let 
        tree =  read test::HTree
        encodes = encodeStrings tree
    putStrLn("Your Huffman tree: " ++ show tree)
    putStrLn("Your encoding paths: " ++ show encodes)
    putStrLn("You can now encode and decode words with your huffman tree, type 'exit' if you want to exit the loop, 'letters' if you want a list of available letters")
    askTree tree encodes

-- Helper function that recurively lets the user parse words in the huffman tree they have loaded.
askTree:: HTree -> [(Char, String)] -> IO()
askTree tree encodes = do
    putStrLn("Enter your next word to parse - or 'exit' or 'letters'") 
    command <- getLine
    if command == "exit"
    then 
        return ()
    else if command == "letters"
    then
        do
            putStr("Your available letters: ")
            putStrLn(intersperse ',' $  map fst encodes) -- kunne også have været map (\x -> fst x) encodes
            askTree tree encodes
    else
        do
            let
                message_encode = encodeMessage command encodes
                message_decode = decodeMessage tree message_encode
            putStrLn("Your encoded message: " ++ message_encode)
            putStrLn("Your decoded message: " ++ message_decode)
            askTree tree encodes
                

                

 -- A 'tuple' data type, that contains an assosiation between a character and an integer (Int) 
type CharAssosiation = (Char, Int)

 -- turns a list of characters (The input string) into an, ordered by largest second element, list of CharAssosiation pairs. 
assosiation :: [Char] -> [CharAssosiation]
assosiation lis = reverse $ sortOn snd $ map (\x -> (head x, length x)) $ group $ sort lis

 -- Datatype for Huffman trees, deriving Show to be 'showable'
 -- derives read as well, in order to read and write HTrees to files.
data HTree = Leaf CharAssosiation | Node  { val :: Int,  left :: HTree, right :: HTree} deriving (Show, Read)

-- Value function, parses a HTree to an integer value
tree_value :: HTree -> Int
tree_value (Leaf t) = snd t
tree_value (Node t1 t2 t3 ) = t1

--Exhaustive functions for merging all permutations of Leafs and Nodes
tree_merge :: HTree -> HTree -> HTree
tree_merge (Leaf a) (Leaf b) = Node (tree_value (Leaf a)  + tree_value (Leaf b)) (Leaf a) (Leaf b)
tree_merge (Leaf a) (Node b1 b2 b3 ) = Node (tree_value (Leaf a)  + b1) (Leaf a)  (Node b1 b2 b3)
tree_merge (Node a1 a2 a3) (Leaf b) = Node (tree_value (Leaf b)  + a1) (Node a1 a2 a3) (Leaf b)
tree_merge (Node a1 a2 a3) (Node b1 b2 b3 ) = Node (a1 + b1) (Node a1 a2 a3) (Node b1 b2 b3)

-- Constructs a huffman tree by recursively calling itself, sorting all available trees for minimum value and combining those two lowest value elements into a node (shortening the tree)
-- Returns when there is one element in the list of HTrees.
construct_htree :: [HTree] -> HTree
construct_htree a  | (length a == 1) = head a
                   | otherwise = let (x:y:xs) = sort_tree a  
                                        in construct_htree(xs ++ [tree_merge (x) (y)] )  

--Construction helper function, helps with readability.
sort_tree :: [HTree] -> [HTree]
sort_tree tree = sortBy(\left right -> compare (tree_value left) (tree_value right)) tree

-- HTree type identification assistant function.
-- Returns true for Nodes, false for Leafs.
is_node :: HTree -> Bool
is_node (Node _ _ _) = True
is_node (Leaf _) = False

-- Path travel assistant function.
-- Throws an error if the char recieved is not 0 or 1.
is_zero :: Char -> Bool
is_zero a   | a == '0' = True
            | a == '1' = False
            | otherwise = error "Wrong Char"

 -- constructs [(Char, String)] pairs of all characters in the huffman code, and their 'binary' path, example:  (e, "00")
 -- it does so by recursively building the path as it travels through Nodes, and appending each Leaf to the output.
encodeStrings :: HTree ->  [(Char, String)]
encodeStrings tree = encodeStringsHelper tree []

-- Removed unnessecary parameters with a nested function
encodeStringsHelper :: HTree -> String -> [(Char, String)]
encodeStringsHelper tree str | is_node tree = let Node _ left right = tree in encodeStringsHelper left (str ++ "0") ++ encodeStringsHelper right (str ++ "1")
                                                | otherwise = let Leaf cha = tree in [(fst cha, str)]                       

 -- takes a message (of conventional chars), and a set of encodings (from encode_strings) and returns a binary message.
 -- if any attempt to look up is not successful (I.E. if a letter does not exist in the huffman tree) then we get an error
 -- if the result is not nothing, then we can turn it an actual string with the unsafe method fromJust, and return that.
encodeMessage :: String -> [(Char, String)] -> String
encodeMessage msg encodings =    let res = concat <$> sequence (map (\x -> (lookUp encodings x)) msg)
                                    in
                                    if isNothing res
                                        then error "Wrong String - the string you input cannot be parsed with the huffman tree"
                                        else fromJust $ res


--Lookup helper function, makes encodeString much more readable - returns a maybe String in this specific usecase, which provides error catching higher up.
--lookUp :: [(Char, String)] -> Char -> Maybe String 
lookUp :: Eq a => [(a, b)] -> a -> Maybe b
lookUp lis key = snd <$> find (\(a,_) -> (a == key)) lis

--Takes the huffman tree, and a binary string of 1's and 0's, returns the decoded message if available.
-- This function fails gracefully / silently
decodeMessage:: HTree -> String -> [Char]
decodeMessage tree str =   let      message = (decodeMessageHelper tree str)
                                    char = fst message 
                                    reducedstr = snd message 
                            in 
                                if null str
                                    then []
                                else char ++ decodeMessage tree reducedstr

-- Helper function that decodes a single substring, I.E. it continues till it hits a leaf on the tree, then returns and gets reset to the top of the tree.
decodeMessageHelper:: HTree -> String -> ([Char], String)
decodeMessageHelper tree str  | is_node tree = let      Node _ left right = tree 
                                                        dir = is_zero (head str)
                                            in
                                                if dir
                                                    then decodeMessageHelper left (tail str)
                                                    else decodeMessageHelper right (tail str)
                            | otherwise = let Leaf cha = tree in ([fst cha], str)
