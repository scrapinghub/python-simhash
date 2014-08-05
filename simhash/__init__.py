"""
Similarity functions for python
"""
# expose public functions natively defined
from _simhash import weighted_fingerprint, fingerprint, hamming_distance, fnvhash

# private functions that are wrapped
from _simhash import similar_indices as _similar_indices

def simpair_indices(sequence_of_hashes, keybits, maxbitdifference=3, rotate=0):
    """simpair_indices

    find the indices of hashes in the sequence passed that differ
    by less than maxbitdifference.
    
    keybits are the number of bits at the start of each hash that 
    we group potential matches by. Set to 0 to find all matches (slower).

    rotate is the number of bits to rotate the hashes by. Only makes sense if
    combined with keybits

    Pairs of results are always returned with the lower index first and 
    there are no duplicate pairs.
    """
    assert maxbitdifference <= 64
    assert keybits <= 64 / maxbitdifference
    assert rotate <= 64
    return _similar_indices(sequence_of_hashes, keybits, maxbitdifference, 
        rotate)
