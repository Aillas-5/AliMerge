# This program is designed to look for merges of Aliquot sequences.

## Version history

* v1.2 (2021-09-02)
    * Initial release on GitHub.
    * Add `Changelog.md` file.
    * Fixed: Use `sscanf` on Linux and `sscanf_s` on Windows.
    * Fixed: Counter `computation only time` displays incorrect information if the application needs to download the OE_3000000_C80.txt file.

* v1.1.1 (2021-08-07)
    * New version published in Mersenne forum in this [post](https://www.mersenneforum.org/showpost.php?p=585093&postcount=1304).
        * Fixed: URL to download the OE_3000000_C80.txt.
        * Fixed: If an C80 was not found for an exponent, the application was stopped.
        * Remove outputs. It was too verbose when checking multiple exponents.

* v1.1.0 (2021-08-07)
    * First opimized version published in Mersenne forum in this [post](https://www.mersenneforum.org/showpost.php?p=585093&postcount=1291).
        * Use Map instead of Vector for faster lookup.
        * Initialize the Map in the loop that load the file to avoid a second loop.
        * Using the map, remove a loop level (From 3 to 2 nested loop).
        * Add time counters (Total/Download/Compute).
        * replace `wget` calls by `curl` calls.

* v1.0.0 (2021-02-16)
    * Initial release on Mersenne forum by EdH in this [post](https://www.mersenneforum.org/showpost.php?p=571769&postcount=794).
