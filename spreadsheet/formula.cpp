#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << "#ARITHM!";
}

namespace {
    class Formula : public FormulaInterface {
    public:       
        explicit Formula(std::string expression) try : ast_(ParseFormulaAST(expression))
        {}
        catch (const std::exception& exc)
        {
            std::throw_with_nested(FormulaException(exc.what()));
        }

        Value Evaluate(const SheetInterface& sheet) const override
        {
            try
            {
                return ast_.Execute(
                    [&sheet](const Position& pos)->double
                    {
                        if (!pos.IsValid())
                        {
                            throw FormulaError(FormulaError::Category::Ref);
                        }
                        const auto* cell = sheet.GetCell(pos);
                        if (!cell)
                        {
                            return 0.0;
                        }
                        auto value = cell->GetValue();
                        
                        if (std::holds_alternative<double>(value))
                        {
                            return std::get<double>(value);
                        }
                        if (std::holds_alternative<std::string>(value))
                        {
                            double result = 0.0;
                            if (!std::get<std::string>(value).empty())
                            {
                                std::istringstream in(std::get<std::string>(value));
                                if (!(in >> result) || !in.eof())
                                {
                                    throw FormulaError(FormulaError::Category::Value);
                                }                                
                            }
                            return result;
                        }
                        throw FormulaError(std::get<FormulaError>(value));
                    }
                );
            }
            catch (const FormulaError& err)
            {
                return err;
            }
        }

        std::string GetExpression() const override
        {
            std::ostringstream out;
            ast_.PrintFormula(out);
            return out.str();
        }

        std::vector<Position> GetReferencedCells() const override
        {
            std::vector<Position> cells;
            for (auto cell : ast_.GetCells())
            {
                if (cell.IsValid())
                {
                    cells.push_back(cell);
                }
            }            
            auto last = std::unique(cells.begin(), cells.end());
            cells.erase(last, cells.end());
            return cells;
        }
                 

    private:
        FormulaAST ast_;
    };
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression)
{
    return std::make_unique<Formula>(std::move(expression));
}