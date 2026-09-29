#include <game/Game.hpp>

#include <app/IniFile.hpp>
#include <app/Settings.hpp>

#include <raylib.h>

#include <algorithm>
#include <charconv>
#include <cstdio>
#include <system_error>

namespace {

// hra-<number>.ini, numbered in the order the games were started.
constexpr std::string_view FILE_PREFIX = "hra-";
constexpr std::string_view FILE_EXTENSION = ".ini";

std::filesystem::path SavesFolder()
{
  return Settings::path().parent_path() / "saves";
}

// The number of the game whose save is `path`, or 0 if it isn't one of ours.
int SaveNumber(const std::filesystem::path& path)
{
  if (path.extension() != FILE_EXTENSION) return 0;
  const std::string stem = path.stem().string();
  if (!stem.starts_with(FILE_PREFIX)) return 0;
  int number = 0;
  const char* first = stem.data() + FILE_PREFIX.size();
  const char* last  = stem.data() + stem.size();
  const auto [end, error] = std::from_chars(first, last, number);
  return error == std::errc() && end == last ? number : 0;
}

std::vector<std::filesystem::path> SaveFiles()
{
  std::vector<std::filesystem::path> files;
  std::error_code error;
  for (std::filesystem::directory_iterator it(SavesFolder(), error), end; !error && it != end; it.increment(error)) {
    if (SaveNumber(it->path()) > 0) files.push_back(it->path());
  }
  return files;
}

// Whether `c` is something a square can have on it. So far only empty.
bool IsSquare(char c)
{
  return c == Game::EMPTY;
}

// The board's rows are row-01 to row-19 of [board], a character a square.
std::string RowKey(int row)
{
  char key[16];
  std::snprintf(key, sizeof(key), "row-%02d", row + 1);
  return key;
}

}  // namespace

Game Game::New(std::string name)
{
  int last = 0;
  for (const std::filesystem::path& file : SaveFiles()) last = std::max(last, SaveNumber(file));
  const int number = last + 1;

  Game game;
  game.name = std::move(name);
  game.path = SavesFolder() / (std::string(FILE_PREFIX) + std::to_string(number) + std::string(FILE_EXTENSION));
  for (auto& row : game.board) row.fill(EMPTY);
  return game;
}

std::optional<Game> Game::Load(const std::filesystem::path& path)
{
  IniFile ini;
  if (!ini.load(path)) return std::nullopt;

  Game game;
  game.path = path;
  game.name = ini.getString("game", "name", path.stem().string());
  for (int y = 0; y < SIZE; ++y) {
    const std::string row = ini.getString("board", RowKey(y), "");
    for (size_t x = 0; x < size_t(SIZE); ++x) {
      const char square = x < row.size() ? row[x] : EMPTY;
      game.board[size_t(y)][x] = IsSquare(square) ? square : EMPTY;
    }
  }
  return game;
}

bool Game::save() const
{
  IniFile ini;
  ini.set("game", "name", this->name);
  ini.set("game", "saved", std::to_string(std::time(nullptr)));
  ini.setInt("board", "size", SIZE);
  for (int row = 0; row < SIZE; ++row) {
    ini.set("board", RowKey(row), std::string(this->board[size_t(row)].begin(), this->board[size_t(row)].end()));
  }
  if (!ini.save(this->path, "A saved game of Vojackova hra.")) {
    TraceLog(LOG_WARNING, "GAME: Couldn't write %s", this->path.string().c_str());
    return false;
  }
  return true;
}

std::vector<SavedGame> ListSavedGames()
{
  std::vector<SavedGame> games;
  for (const std::filesystem::path& file : SaveFiles()) {
    IniFile ini;
    if (!ini.load(file)) continue;
    SavedGame game;
    game.path = file;
    game.name = ini.getString("game", "name", file.stem().string());
    const std::string saved = ini.getString("game", "saved", "0");
    long long seconds = 0;
    std::from_chars(saved.data(), saved.data() + saved.size(), seconds);
    game.saved = std::time_t(seconds);
    games.push_back(std::move(game));
  }
  std::sort(games.begin(), games.end(), [](const SavedGame& a, const SavedGame& b) { return a.saved > b.saved; });
  return games;
}

bool DeleteSave(const std::filesystem::path& path)
{
  std::error_code error;
  std::filesystem::remove(path, error);
  if (error) TraceLog(LOG_WARNING, "GAME: Couldn't delete %s", path.string().c_str());
  return !error;
}

std::string SaveTimeText(std::time_t time)
{
  std::tm local{};
  if (localtime_s(&local, &time) != 0) return {};
  char text[32];
  std::snprintf(text, sizeof(text), "%d. %d. %d %d:%02d", local.tm_mday, local.tm_mon + 1, local.tm_year + 1900,
                local.tm_hour, local.tm_min);
  return text;
}
