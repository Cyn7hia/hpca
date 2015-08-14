**hpca** provides an efficient implementation of the Hellinger PCA for computing word embeddings.
See the [EACL 2014 paper](http://lebret.ch/wp-content/uploads/2014/03/eacl2014.pdf) for more details.

## PREREQUISITES

 This project requires:
  * Cross-platform Make (CMake) v2.6+
  * GNU Make or equivalent.
  * GCC or an alternative, reasonably conformant C++ compiler.
  * Zlib v1.2.5
  * OpenMP API (optional)
  * Doxygen (in order to make documentation which is optional)

## BUILDING
 
 This project uses the Cross-platform Make (CMake) build system. However, we
 have conveniently provided a wrapper configure script and Makefile so that
 the typical build invocation of `./configure` followed by `make` will work.
 For a list of all possible build targets, use the command `make help`.

 **NOTE**: Users of CMake may believe that the top-level Makefile has been
 generated by CMake; it hasn't, so please do not delete that file.

## INSTALLING

 Once the project has been built (see "BUILDING"), execute `sudo make install`.
 
 See [Install](INSTALL.md) for more details.

## GETTING WORD EMBEDDINGS

This package includes 6 different tools: `preprocess`, `vocab`, `cooccurrence`, `pca`, `embeddings` and `eval`.

### Corpus preprocessing 

Lowercase conversion and/or all numbers replaced with a special token ('0').


The corpus needs to be a **tokenized** plain text file containing only the **sentences** of the corpus.

Before running the `preprocess` tool, authors strongly recommend to follow these two steps:

1) Running a sentence detector, e.g. [the Apache OpenNLP Sentence Dectector](https://opennlp.apache.org/documentation/1.5.3/manual/opennlp.html#tools.sentdetect).
```
./apache-opennlp-1.5.3/bin/opennlp SentenceDetector ./apache-opennlp-1.5.3/bin/en-sent.bin < corpus.txt > corpus-sentences.txt
```

2) Running a tokenizer, e.g. [the Stanford Tokenizer](http://nlp.stanford.edu/software/tokenizer.shtml).
```
java -cp stanford-parser.jar edu.stanford.nlp.process.PTBTokenizer -preserveLines corpus-sentences.txt > corpus-token.txt
```

`preprocess` options:
* `-lower <int>`: Lowercased? 0=off or 1=on (default)
* `-digit <int>`: Replace all digits with a special token? 0=off or 1=on (default)
* `-input-file <file>`: Input file to preprocess (gzip format is allowed)
* `-output-file <file>`: Output file to save preprocessed data
* `-gzip <int>`: Save in gzip format? 0=off (default) or 1=on
* `-threads <int>`: Number of threads; default 8
* `-verbose <int>`: Set verbosity: 0=off or 1=on (default)

**Example**:
```
preprocess -input-file corpus-token.txt -output-file corpus-clean.txt -lower 1 -digit 1 -verbose 1 -threads 8 -gzip 0
```

### Vocabulary extraction

Extracting words with their respective frequency.

`vocab` options:
* `-input-file <file>`: Input file from which to extract the vocabulary (gzip format is allowed)
* `-vocab-file <file>`: Output file to save the vocabulary
* `-threads <int>`: Number of threads; default 8
* `-verbose <int>`: Set verbosity:  0=off or 1=on (default)

**Example**:
```
vocab -input-file corpus-clean.txt -vocab-file vocab.txt -nthread 8 -verbose 1
```

### Getting co-occurrence probability matrix

Constructing word-word cooccurrence statistics from the corpus. 
The user should supply a vocabulary file, as produced by `vocab`.

`cooccurrence` options:
* `-input-file <file>`: Input file containing the tokenized and cleaned corpus text (gzip format is allowed)
* `-vocab-file <file>`: Vocabulary file
* `-output-dir <dir>`: Output directory name to save files
* `-min-freq <int>`: Discarding all words with a lower appearance frequency (default is 100)
* `-upper-bound <float>`: Discarding words from the context vocabulary with a upper appearance frequency (default is 1.0)
* `-lower-bound <float>`: Discarding words from the context vocabulary with a lower appearance frequency (default is 0.00001)
* `-cxt-size <int>`: Symmetric context size around words(default is 5)
* `-dyn-cxt <int>`: Dynamic context window, i.e. weighting by distance form the focus word: 0=off (default) or 1=on
* `-memory <float>`: Soft limit for memory consumption in GB; default 4.0
* `-threads <int>`: Number of threads; default 8
* `-verbose <int>`: Set verbosity:  0=off or 1=on (default)

**Example**:
```
cooccurrence -input-file corpus-clean.txt -vocab-file vocab.txt -output-dir path_to_dir -min-freq 100 -cxt-size 5 -dyn-cxt 1 -memory 4.0 -upper-bound 1.0 -lower-bound 0.00001 -verbose 1 -threads 8
```

`cooccurence` will create the following files into the directory specified by the `-output-dir` option:
* `coccurrence.bin`: binary file containing the counts
* `target_words.txt`: vocabulary of words from which embeddings will be generated (rows of the cooccurrence matrix)
* `context_words.txt`: vocabulary of context words (columns of the cooccurrence matrix)

## Performing Hellinger PCA

Randomized SVD with respect to the Hellinger distance.
The user should supply the directory where files produced by `cooccurrence` are.

Let `A` be a sparse matrix to be analyzed with `n` rows and `m` columns, and `r` be the ranks of a truncated SVD (with `r < min(n,m)`).
Formally, the SVD of `A` is a factorization of the form `A = U S Vᵀ`.

Unfortunately, computing the SVD can be extremely time-consuming as `A` is often a very large matrix. Thus, we turn to randomized methods which offer significant speedups over classical methods.
This tool use some modern randomized matrix approximation techniques, developed in (amongst others) [Finding structure with randomness: Probabilistic algorithms for constructing approximate matrix decompositions](http://arxiv.org/abs/0909.4061), a 2009 paper by Nathan Halko, Per-Gunnar Martinsson and Joel A. Tropp.

This tool uses the external [redsvd](https://code.google.com/p/redsvd/) library, which implements this randomized SVD using [Eigen3](http://eigen.tuxfamily.org).

`pca` options:
* `-input-dir <dir>`: Directory where to find the `cooccurrence.bin` file
* `-rank <int>`: Number of components to keep; default 300
* `-threads <int>`: Number of threads; default 8
* `-verbose <int>`: Set verbosity: 0=off or 1=on (default)

**Example**:
```
pca -input-dir path_to_cooccurence_file -rank 300
```

`pca` will create the following files into the directory specified by the `-input-dir` option:
* `svd.U`: orthonomal matrix U
* `svd.S`: diagonal matrix S whose entries are singular values
* `svd.V`: orthonomal matrix V


## Extracting word embeddings

Generating word embeddings from the Hellinger PCA.
The user should supply the directory where files produced by `pca` are.

`embeddings` options:
* `-input-dir <dir>`: Directory where to find files from the randomized SVD
* `-output-name <string>`: Filename for embeddings file which will be placed in <dir> (default is words.txt)
* `-dim <int>`: Word vector dimension; default 100
* `-eig <float>`: Eigenvalue weighting (0.0, 0.5 or 1.0); default is 0.0
* `-norm <int>`: Are vectors normalized to unit length? 0=off (default) or 1=on

**Example**:
```
embeddings -input-dir path_to_svd_files -output-name words.txt -eig 0.0 -dim 100 -norm 0
```

## Evaluating word embeddings

This tool provides a quick evaluation of the word embeddings produced by `embeddings`.

It contains the following evaluation datasets:
* [The WordSimilarity-353 Test Collection](http://www.cs.technion.ac.il/~gabr/resources/data/wordsim353/)
* [The Rubenstein and Goodenough dataset](http://dl.acm.org/citation.cfm?id=365657)
* [The Stanford Rare Word (RW) Similarity Dataset](http://www-nlp.stanford.edu/~lmthang/morphoNLM/)
* [The Microsoft Research Syntactic Analogies Dataset](http://aclweb.org/aclwiki/index.php?title=Syntactic_Analogies_(State_of_the_art))
* [The Google Semantic Analogies Dataset](http://arxiv.org/abs/1301.3781)

`eval` options:
* `-input-file <file>`: File containing word embeddings to evaluate
* `-vocab-file <file>`: File containing word vocabulary
* `-ws353 <int>`: Do WordSim-353 evaluation: 0=off or 1=on (default)
* `-rg65 <int>`: Do Rubenstein and Goodenough 1965 evaluation: 0=off or 1=on (default)
* `-rw <int>`: Do Stanford Rare Word evaluation: 0=off or 1=on (default)
* `-syn <int>`: Do Microsoft Research Syntactic Analogies: 0=off or 1=on (default)
* `-sem <int>`: Do Google Semantic Analogies: 0=off or 1=on (default)
* `-verbose <int>`: Set verbosity: 0=off or 1=on (default)

**Example**:
```
eval -input-file words.txt -vocab-file target_vocab.txt -ws353 1 -rg65 1 -rw 1 -syn 1 -sem 1 -verbose 1
```

**NOTE**: To speed up the implementation of the analogies, candidate solutions come from a closed vocabulary.

## AUTHORS 

 * Rémi Lebret: remi@lebret.ch

## ACKNOWLEDGEMENTS

  * Eigen3 -- http://eigen.tuxfamily.org

  Eigen 3 is a lightweight C++ template library for vector and matrix math, a.k.a. linear algebra.
  

  * redsvd -- https://code.google.com/p/redsvd/

  redsvd is a library for solving several matrix decompositions including singular value decomposition (SVD), principal component analysis (PCA), and eigen value decomposition.
  redsvd uses Eigen3.


