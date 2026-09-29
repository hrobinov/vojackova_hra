#include <app/IniFile.hpp>

#include <charconv>
#include <fstream>
#include <system_error>

namespace {

std::string_view Trim(std::string_view s)
{
  constexpr std::string_view SPACE = " \t\r\n";
  const size_t first = s.find_first_not_of(SPACE);
  if (first == std::string_view::npos) return {};
  return s.substr(first, s.find_last_not_of(SPACE) - first + 1);
}

}  // namespace

bool IniFile::load(const std::filesystem::path& path)
{
  std::ifstream in(path);
  if (!in) return false;

  this->sections.clear();
  std::string current;  // keys before any [section] go in the unnamed one
  std::string line;
  while (std::getline(in, line)) {
    const std::string_view text = Trim(line);
    if (text.empty() || text[0] == ';' || text[0] == '#') continue;

    if (text.front() == '[' && text.back() == ']') {
      current = Trim(text.substr(1, text.size() - 2));
      continue;
    }

    const size_t eq = text.find('=');
    if (eq == std::string_view::npos) continue;
    const std::string_view key = Trim(text.substr(0, eq));
    if (key.empty()) continue;
    this->set(current, key, std::string(Trim(text.substr(eq + 1))));
  }
  return true;
}

bool IniFile::save(const std::filesystem::path& path, std::string_view comment) const
{
  std::error_code error;
  if (path.has_parent_path()) std::filesystem::create_directories(path.parent_path(), error);

  std::filesystem::path temp = path;
  temp += ".tmp";
  {
    std::ofstream out(temp, std::ios::trunc);
    if (!out) return false;

    while (!comment.empty()) {
      const size_t end = comment.find('\n');
      out << "; " << comment.substr(0, end) << '\n';
      comment = end == std::string_view::npos ? std::string_view{} : comment.substr(end + 1);
    }

    for (const Section& section : this->sections) {
      if (section.entries.empty()) continue;
      if (!section.name.empty()) out << "\n[" << section.name << "]\n";
      for (const auto& [key, value] : section.entries) out << key << " = " << value << '\n';
    }
    if (!out.flush()) return false;
  }

  std::filesystem::rename(temp, path, error);
  return !error;
}

std::string IniFile::getString(std::string_view section, std::string_view key, std::string_view fallback) const
{
  const std::string* value = this->find(section, key);
  return std::string(value ? std::string_view(*value) : fallback);
}

int IniFile::getInt(std::string_view section, std::string_view key, int fallback) const
{
  const std::string* value = this->find(section, key);
  if (!value) return fallback;
  int result = 0;
  const char* end = value->data() + value->size();
  const auto [ptr, error] = std::from_chars(value->data(), end, result);
  return error == std::errc() && ptr == end ? result : fallback;
}

bool IniFile::getBool(std::string_view section, std::string_view key, bool fallback) const
{
  const std::string* value = this->find(section, key);
  if (!value) return fallback;
  if (*value == "true" || *value == "1") return true;
  if (*value == "false" || *value == "0") return false;
  return fallback;
}

std::vector<std::pair<std::string, std::string>> IniFile::entries(std::string_view section) const
{
  for (const Section& s : this->sections) {
    if (s.name == section) return s.entries;
  }
  return {};
}

void IniFile::set(std::string_view section, std::string_view key, std::string value)
{
  Section& s = this->sectionNamed(section);
  for (auto& [k, v] : s.entries) {
    if (k == key) { v = std::move(value); return; }
  }
  s.entries.emplace_back(std::string(key), std::move(value));
}

void IniFile::setInt(std::string_view section, std::string_view key, int value)
{
  this->set(section, key, std::to_string(value));
}

void IniFile::setBool(std::string_view section, std::string_view key, bool value)
{
  this->set(section, key, value ? "true" : "false");
}

const std::string* IniFile::find(std::string_view section, std::string_view key) const
{
  for (const Section& s : this->sections) {
    if (s.name != section) continue;
    for (const auto& [k, v] : s.entries) {
      if (k == key) return &v;
    }
  }
  return nullptr;
}

IniFile::Section& IniFile::sectionNamed(std::string_view name)
{
  for (Section& s : this->sections) {
    if (s.name == name) return s;
  }
  this->sections.push_back(Section{ std::string(name), {} });
  return this->sections.back();
}
