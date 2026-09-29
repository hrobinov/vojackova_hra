// A minimal INI file: [sections] of `key = value` lines, with ; or # comments.
// Sections and keys stay in the order they were read or set, so a saved file
// reads the way it was written.

#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

class IniFile {
public:
  // False if the file can't be read. Lines that don't parse are skipped.
  bool load(const std::filesystem::path& path);

  // Creates the folder if need be; `comment` goes at the top as ; lines.
  // Writes a temporary file and renames it over the old one, so a crash
  // mid-save can't leave half a file. False if it couldn't be written.
  bool save(const std::filesystem::path& path, std::string_view comment = {}) const;

  // `fallback` when the key is missing or its value doesn't parse.
  std::string getString(std::string_view section, std::string_view key, std::string_view fallback) const;
  int         getInt(std::string_view section, std::string_view key, int fallback) const;
  bool        getBool(std::string_view section, std::string_view key, bool fallback) const;
  // Every key of `section` with its value, in order; none if there is no such section.
  std::vector<std::pair<std::string, std::string>> entries(std::string_view section) const;

  void set(std::string_view section, std::string_view key, std::string value);
  void setInt(std::string_view section, std::string_view key, int value);
  void setBool(std::string_view section, std::string_view key, bool value);

private:
  struct Section {
    std::string                                      name;
    std::vector<std::pair<std::string, std::string>> entries;
  };

  const std::string* find(std::string_view section, std::string_view key) const;
  Section&           sectionNamed(std::string_view name);

  std::vector<Section> sections;
};
