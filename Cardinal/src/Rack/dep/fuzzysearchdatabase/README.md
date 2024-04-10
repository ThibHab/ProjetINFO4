# Fuzzy Search Database #

Copyright 2022 Nils Jonas Norberg <jnorberg@gmail.com>

License: BSD-3-Clause ( https://opensource.org/licenses/BSD-3-Clause )

# What is This #
A simple "fuzzy" search library. Single hpp-file implementation.

# Usage #
1. Create a ```fuzzysearch::Database<>```
2. Add entries ( with "fields" ) ```void addEntry( Key key, const std::vector<std::string>& fields )```
3. (optionally) Set Weights per Field ```void setWeights( const std::vector<float>& fieldWeights )```
4. (optionally) Set Threshold ```void setThreshold( float threshold )```
5. Search with ```std::vector< Result > search( std::string queryString ) const```


# Setup #
1. Start Msys2
2. run buildInMsys2.sh
3. run testFuzzy.exe
