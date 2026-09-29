#pragma once
#include <set>
#include <vector>
namespace agui { class Table; }

namespace agui
{
  class TableConnector
  {
  public:
    void logic() { this->columnWidths.clear(); }
    void add(Table* table);
    void remove(Table* table);
    bool isResizeResolved() const { return !this->columnWidths.empty(); }
    void resolveResize();
    const std::vector<int>& getColumnWidths() const { return this->columnWidths; }

  private:
    std::set<Table*> tables;
    std::vector<int> columnWidths;
  };
}
