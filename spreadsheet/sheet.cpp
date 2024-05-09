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
    CheckPos(pos);
    table_.resize(std::max(pos.row + 1, int(std::size(table_))));
    table_[pos.row].resize(std::max(pos.col + 1, int(std::size(table_))));
    if (!table_[pos.row][pos.col])
    {
        table_[pos.row][pos.col] = std::make_unique<Cell>(*this);
    }
    table_[pos.row][pos.col]->Set(text);
}

const CellInterface* Sheet::GetCell(Position pos) const
{
    CheckPos(pos);
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
    CheckPos(pos);
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
    CheckPos(pos);
    if (IsCellValid(pos))
    {
        if (table_[pos.row][pos.col])
        {
            table_[pos.row][pos.col].reset();
        }
    }
}

Size Sheet::GetPrintableSize() const
{
    Size result{ 0, 0 };
    for (size_t i = 0; i < table_.size(); ++i)
    {
        for (size_t j = 0; j < table_[i].size(); ++j)
        {
            if (table_[i][j])
            {
                if (table_[i][j]->GetText().empty())
                {
                    continue;
                }  
                result.rows = std::max(size_t(result.rows), i + 1);
                result.cols = std::max(size_t(result.cols), j + 1);
            }
        }
    }

    return result;
}


void Sheet::PrintValues(std::ostream& output) const
{
    for (size_t i = 0; i < size_t(GetPrintableSize().rows); ++i)
    {
        for (size_t j = 0; j < size_t(GetPrintableSize().cols); ++j)
        {
            if (j > 0)
            {
                output << '\t';
            }
            if (j < table_[i].size())
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
    for (size_t i = 0; i < size_t(GetPrintableSize().rows); ++i)
    {
        for (size_t j = 0; j < size_t(GetPrintableSize().cols); ++j)
        {
            if (j)
            {
                output << '\t';
            }
            if (j < table_[i].size())
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

void Sheet::CheckPos(Position pos) const
{
    if (!pos.IsValid())
    {
        throw InvalidPositionException("Invalid pos");
    }
}

std::unique_ptr<SheetInterface> CreateSheet()
{
    return std::make_unique<Sheet>();
}