from __future__ import annotations

import os
import re
import string

from .mpq import MPQArchive


PATCH_SUFFIXES = [str(n) for n in range(2, 10)] + list(string.ascii_uppercase)


BASE_ARCHIVES = ["common.MPQ", "common-2.MPQ", "expansion.MPQ", "lichking.MPQ",
                 "patch.MPQ"]
LOCALE_ARCHIVES = ["base-{loc}.MPQ", "locale-{loc}.MPQ", "speech-{loc}.MPQ",
                   "expansion-locale-{loc}.MPQ", "expansion-speech-{loc}.MPQ",
                   "lichking-locale-{loc}.MPQ", "lichking-speech-{loc}.MPQ",
                   "patch-{loc}.MPQ"]

LOCALE_DIR_RE = re.compile(r"^[a-z]{2}[A-Z]{2}$")


def detect_locales(data_dir):

    found = []
    try:
        entries = sorted(os.listdir(data_dir))
    except OSError:
        return found
    for name in entries:
        if LOCALE_DIR_RE.match(name) and os.path.isdir(os.path.join(data_dir, name)):
            found.append(name)
    return found


def _resolve(directory, filename):

    candidate = os.path.join(directory, filename)
    if os.path.isfile(candidate):
        return candidate
    try:
        entries = os.listdir(directory)
    except OSError:
        return None
    lowered = filename.lower()
    for entry in entries:
        if entry.lower() == lowered:
            path = os.path.join(directory, entry)
            if os.path.isfile(path):
                return path
    return None


def archive_chain(data_dir, locale):

    locale_dir = os.path.join(data_dir, locale)

    low_to_high = []
    for name in BASE_ARCHIVES:
        path = _resolve(data_dir, name)
        if path:
            low_to_high.append(path)
    for suffix in PATCH_SUFFIXES:
        path = _resolve(data_dir, "patch-%s.MPQ" % suffix)
        if path:
            low_to_high.append(path)

    if os.path.isdir(locale_dir):
        for template in LOCALE_ARCHIVES:
            path = _resolve(locale_dir, template.format(loc=locale))
            if path:
                low_to_high.append(path)
        for suffix in PATCH_SUFFIXES:
            path = _resolve(locale_dir, "patch-%s-%s.MPQ" % (locale, suffix))
            if path:
                low_to_high.append(path)

    return list(reversed(low_to_high))


def free_patch_suffix(data_dir, locales):






    for suffix in reversed(string.ascii_uppercase):
        if _resolve(data_dir, "patch-%s.MPQ" % suffix):
            continue
        clash = False
        for locale in locales:
            locale_dir = os.path.join(data_dir, locale)
            if os.path.isdir(locale_dir) and _resolve(
                    locale_dir, "patch-%s-%s.MPQ" % (locale, suffix)):
                clash = True
                break
        if not clash:
            return suffix
    return None


class ClientFiles:








    def __init__(self, data_dir, locale, exclude=None):
        self.data_dir = data_dir
        self.locale = locale
        drop = {x.lower() for x in (exclude or ())}
        self.chain = [p for p in archive_chain(data_dir, locale)
                      if os.path.basename(p).lower() not in drop]
        self._open = {}

    def close(self):
        for archive in self._open.values():
            archive.close()
        self._open.clear()

    def __enter__(self):
        return self

    def __exit__(self, *exc):
        self.close()

    def _archive(self, path):
        if path not in self._open:
            self._open[path] = MPQArchive(path)
        return self._open[path]

    def find(self, name):

        errors = []
        for path in self.chain:
            try:
                archive = self._archive(path)
            except Exception as exc:
                errors.append("%s: %s" % (os.path.basename(path), exc))
                continue
            try:
                if archive.has_file(name):
                    return archive.read_file(name), path
            except Exception as exc:
                errors.append("%s: %s" % (os.path.basename(path), exc))
        raise FileNotFoundError(
            "%s not found in any client archive%s"
            % (name, ("\n  " + "\n  ".join(errors)) if errors else ""))
