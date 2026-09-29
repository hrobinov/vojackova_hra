// Turns a file into a C++ source defining it as a byte array, so the exe can
// carry its assets instead of loading them from next to itself.
//   Embed <input> <output.cpp> <SYMBOL> <namespace>
// defines `const unsigned char <SYMBOL>[]` and `const std::size_t <SYMBOL>_SIZE`
// in <namespace>. Built and run by fbuild.bff.
#include <cstdio>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

int main(int argc, char** argv) {
  if (argc != 5) {
    std::fprintf(stderr, "usage: Embed <input> <output.cpp> <SYMBOL> <namespace>\n");
    return 1;
  }
  const std::string input = argv[1], output = argv[2], symbol = argv[3], ns = argv[4];

  std::ifstream in(input, std::ios::binary);
  if (!in) {
    std::fprintf(stderr, "Embed: can't read %s\n", input.c_str());
    return 1;
  }
  const std::vector<unsigned char> bytes{std::istreambuf_iterator<char>(in), {}};

  std::string name = input.substr(input.find_last_of("/\\") + 1);
  std::ostringstream out;
  out << "// Generated from " << name << " by tools/Embed.cpp -- don't edit.\n"
      << "#include <cstddef>\n\nnamespace " << ns << " {\n\n"
      << "extern const unsigned char " << symbol << "[] = {\n";
  static const char hex[] = "0123456789abcdef";
  for (size_t i = 0; i < bytes.size(); ++i) {
    out << "0x" << hex[bytes[i] >> 4] << hex[bytes[i] & 15] << ',';
    if (i % 32 == 31) out << '\n';
  }
  out << "\n};\nextern const std::size_t " << symbol << "_SIZE = " << bytes.size() << ";\n\n"
      << "}  // namespace " << ns << "\n";

  std::ofstream(output, std::ios::binary) << out.str();
  return 0;
}
