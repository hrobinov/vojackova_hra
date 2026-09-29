#include "Agui/TableConnector.hpp"
#include "Agui/Widget/Table.hpp"
#include <algorithm>
#include <stdexcept>

namespace agui
{
  void TableConnector::add(Table* table)
  {
    this->tables.insert(table);
  }

  void TableConnector::remove(Table* table)
  {
    this->tables.erase(table);
    if (this->tables.empty())
      delete this;
  }

  void TableConnector::resolveResize()
  {
    if (!this->columnWidths.empty())
      return;
    for (Table* table : this->tables)
    {
      // this is redundant resize for some of the table contents
      // TODO: solve by not doing extra resizes.
      for (Widget* widget : table->getChildren())
        widget->resizeToContentsRecursive();
      if (this->columnWidths.empty())
        this->columnWidths = table->getColumnWidths(table->getColumnCount());
      else
      {
        std::vector<int> other = table->getColumnWidths(table->getColumnCount());
        if (other.empty())
          continue;
        this->columnWidths.resize(std::max(this->columnWidths.size(), other.size()));
        for (uint32_t i = 0; i < std::max(this->columnWidths.size(), other.size()); ++i)
          this->columnWidths[i] = std::max(this->columnWidths.size() > i ? this->columnWidths[i] : 0, other.size() > i ? other[i] : 0);
      }
    }
  }
}
