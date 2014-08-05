=======
simhash
=======

This is an efficient implementation of some functions that are useful for implementing near duplicate detection based on `Charikar's simhash <http://www.cs.princeton.edu/courses/archive/spring04/cos598B/bib/CharikarEstim.pdf>`_. It is a python module, written in C with GCC extentions, and includes the following functions:

`fingerprint`
    Generates a fingerprint from a sequence of hashes

`weighted_fingerprint`
    Generate a fingerprint from a sequence of (long long, weight) tuples

`fnvhash`
    Generates a (FNV-1a) hash from a string

`hamming_distance`
    Calculate the number of bits that differ between 2 long long integers

`simpair_indices`
    Find the indices of hashes in a sequence that differ by less than a certain number of bits. It includes arguments for rotating and grouping hashes. It can be used to help efficiently implement online or batch near duplicate detection, for example as described in `Detecting Near-Duplicates for Web Crawling <http://www.wwwconference.org/www2007/papers/paper215.pdf>`_ by Gurmeet Manku, Arvind Jain, and Anish Sarma.


Example usage
-------------

Generate hashes::

    >>> from simhash import fingerprint
    >>> hash1 = fingerprint(map(hash, "some text we want to hash"))
    >>> hash2 = fingerprint(map(hash, "some more text we want to hash"))

Measure distance between hashes::

    >>> from simhash import hamming_distance
    >>> hamming_distance(hash1, hash2)
    2L

This code was used from mapreduce jobs against a large dataset of webpages as part of a prototype at Scrapinghub.
