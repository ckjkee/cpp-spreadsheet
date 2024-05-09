#pragma once

#include "common.h"
#include "formula.h"

#include <functional>
#include <unordered_set>

class Sheet;

class Cell : public CellInterface {
public:
    Cell(Sheet& sheet);
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;
    

    bool IsReferenced() const;

private:
    class Impl;
    class EmptyImpl;
    class TextImpl;
    class FormulaImpl;

    void InvalidateCache();

    void InvalidateCell();

    bool IsCached() const;

    bool IsCyclicDependent(const Impl& new_impl) const;

    void ValidateDependencies();

    Sheet& sheet_;
    std::unique_ptr<Impl> impl_;
    std::unordered_set<Cell*> left_nodes_;
    std::unordered_set<Cell*> right_nodes_;

};
