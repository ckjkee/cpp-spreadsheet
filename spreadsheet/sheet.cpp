#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text)
{
    if (!pos.IsValid())
    {
        throw InvalidPositionException("Invalid pos");
    }
    else
    {
        table_.resize(std::max(pos.row + 1, int(std::size(table_))));
        table_[pos.row].resize(std::max(pos.col + 1, int(std::size(table_))));

        if (!table_[pos.row][pos.col])
        {
            table_[pos.row][pos.col] = std::make_unique<Cell>(*this);
        }

        table_[pos.row][pos.col]->Set(text);
    }

}

const CellInterface* Sheet::GetCell(Position pos) const
{
    if (!pos.IsValid())
    {
        throw InvalidPositionException("Invalid pos");
    }

    if (IsCellValid(pos))
    {
        if (table_[pos.row][pos.col])
        {
            return table_[pos.row][pos.col].get();
        }
    }
    return nullptr;
}

CellInterface* Sheet::GetCell(Position pos)
{
    if (!pos.IsValid())
    {
        throw InvalidPositionException("Invalid pos");
    }
    
        if (IsCellValid(pos))
        {
            if (table_[pos.row][pos.col])
            {
                return table_[pos.row][pos.col].get();
            }            
        }
        return nullptr;
}


void Sheet::ClearCell(Position pos)
{
    if (!pos.IsValid())
    {
        throw InvalidPositionException("Invalid pos");
    }
    else
    {
        if (IsCellValid(pos))
        {
            if (table_[pos.row][pos.col])
            {
                table_[pos.row][pos.col].reset();
            }
        }
    }
}

Size Sheet::GetPrintableSize() const
{
    Size result{ 0, 0 };

    for (int i = 0; i < int(table_.size()); ++i)
    {
        for (int j = int(table_[i].size() - 1); j >= 0; --j)
        {
            if (table_[i][j])
            {
                if (table_[i][j].get()->GetText().empty())
                {
                    continue;
                }
                else
                {
                    result.rows = std::max(result.rows, i + 1);
                    result.cols = std::max(result.cols, j + 1);
                }
            }
        }
    }

    return result;
}

void Sheet::PrintValues(std::ostream& output) const
{
    for (int i = 0; i < GetPrintableSize().rows; ++i)
    {
        for (int j = 0; j < GetPrintableSize().cols; ++j)
        {
            if (j > 0)
            {
                output << '\t';
            }
            if (j < int(table_[i].size()))
            {
                if (table_[i][j])
                {
                    std::visit([&output](const auto& val) {output << val; }, table_[i][j]->GetValue());
                }
            }
        }
        output << '\n';
    }
}

void Sheet::PrintTexts(std::ostream& output) const
{
    for (int i = 0; i < GetPrintableSize().rows; ++i)
    {
        for (int j = 0; j < GetPrintableSize().cols; ++j)
        {
            if (j)
            {
                output << '\t';
            }
            if (j < int(table_[i].size()))
            {
                if (table_[i][j])
                {
                    output << table_[i][j]->GetText();
                }
            }
        }
        output << '\n';
    }
}

bool Sheet::IsCellValid(Position pos) const
{
    return pos.row < int(table_.size()) && pos.col < int(table_[pos.row].size());
}

std::unique_ptr<SheetInterface> CreateSheet()
{
    return std::make_unique<Sheet>();
}