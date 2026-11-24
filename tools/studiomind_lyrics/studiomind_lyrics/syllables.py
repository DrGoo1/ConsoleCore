"""Offline English syllable counting.

This is a heuristic, not a dictionary lookup. It is used as a *gate*: the
model splits every line into syllables itself, and this counter checks that
split independently. Lines where the two disagree by more than the tolerance
are sent back to the model once (see generate.py).
"""

from __future__ import annotations

import re

# Words the vowel-group rules get wrong. Lower-case, letters and apostrophes only.
_EXCEPTIONS = {
    "fire": 1, "fires": 1, "hour": 1, "hours": 1, "our": 1, "flour": 1,
    "every": 2, "everything": 3, "everyone": 3, "evening": 2, "different": 2,
    "family": 3, "chocolate": 2, "camera": 3, "interest": 2, "business": 2,
    "poem": 2, "poems": 2, "quiet": 2, "lion": 2, "being": 2, "seeing": 2,
    "doing": 2, "going": 2, "saying": 2, "playing": 2, "riot": 2,
    "create": 2, "created": 3, "react": 2, "idea": 3, "ideas": 3,
    "real": 1, "really": 2, "heaven": 2, "seven": 2, "eleven": 3,
    "people": 2, "little": 2, "simple": 2, "table": 2, "trouble": 2,
    "maybe": 2, "somewhere": 2, "someone": 2, "something": 2, "sometimes": 2,
    "whatever": 3, "forever": 3, "never": 2, "over": 2, "naive": 2,
    "the": 1, "are": 1, "were": 1, "there": 1, "where": 1, "here": 1,
    "fire's": 1, "i'm": 1, "you're": 1, "we're": 1, "they're": 1,
    "don't": 1, "can't": 1, "won't": 1, "isn't": 2, "wasn't": 2,
    "doesn't": 2, "didn't": 2, "couldn't": 2, "wouldn't": 2, "shouldn't": 2,
    "breathe": 1, "clothes": 1, "smile": 1, "while": 1, "whole": 1,
    "alive": 2, "inside": 2, "outside": 2, "goodbye": 2, "tonight": 2,
    "wildfire": 2, "heartbeat": 2, "lifetime": 2, "somehow": 2,
    "tired": 1, "desire": 2, "higher": 2, "via": 2, "piano": 3,
}

_VOWELS = "aeiouy"
_WORD_RE = re.compile(r"[a-z']+")


def words(text: str) -> list[str]:
    """Lower-case word tokens, apostrophes kept, digits and punctuation dropped."""
    return _WORD_RE.findall(text.lower().replace("’", "'"))


def count_word(word: str) -> int:
    """Estimate syllables in one word (at least 1 for any word with letters)."""
    w = word.lower().strip("'")
    if not w:
        return 0
    if w in _EXCEPTIONS:
        return _EXCEPTIONS[w]
    if w.endswith("'s"):
        w = w[:-2]
    w = w.replace("'", "")

    groups = re.findall(r"[aeiouy]+", w)
    n = len(groups)

    # Silent final e: "love", "time" (but not "the", "be", "people"-style "le").
    if w.endswith("e") and not w.endswith(("le", "ee", "ye")) and n > 1:
        n -= 1
    # "-le" after a consonant is its own syllable: "ta-ble", "lit-tle".
    if w.endswith("le") and len(w) > 2 and w[-3] not in _VOWELS:
        if not (n > 0 and groups[-1] == "e"):
            n += 1
    # Past tense "-ed" is silent unless after t or d: "walked" vs "waited".
    if w.endswith("ed") and len(w) > 3 and w[-3] not in "td" and n > 1:
        n -= 1
    # "-es" is silent unless after s, x, z, ch, sh, ge, ce: "times" vs "boxes".
    if w.endswith("es") and len(w) > 3 and n > 1:
        if not (w[-3] in "sxz" or w.endswith(("ches", "shes", "ges", "ces"))):
            n -= 1
    # Silent e before a suffix: "lone-ly", "time-less", "care-ful", "some-day".
    n -= len(re.findall(r"[^aeiouy]e(?:ly|less|ful|ment|ness|day|times|where|one|thing|body|way|coming|light|line|side|town|land|ward|wards)$", w))
    # Vowel pairs that split: "-ia", "-io", "-ea" in "area", "create" handled above.
    n += len(re.findall(r"(?:ia|io|eo|ua|uo)(?![u])", w)) if len(w) > 4 else 0
    return max(1, n)


def count_line(text: str) -> int:
    return sum(count_word(w) for w in words(text))
