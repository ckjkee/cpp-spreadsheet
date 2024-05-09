#include "cell.h"
#include "sheet.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include <set>

class Cell::Impl
{
public:
	virtual Value GetValue() const = 0;
	virtual std::string GetText() const = 0;
	virtual bool IIsCached() const { return true; }
	virtual void IInvalidateCache() {}
	virtual std::vector<Position> IGetReferencedCells() const { return {}; }

protected:
	std::string text_;
	Value value_;
};

class Cell::EmptyImpl : public Impl {
public:
	EmptyImpl()
	{
		value_ = "";
		text_ = "";
	}

	Value GetValue() const
	{
		return value_;
	}

	std::string GetText() const
	{
		return text_;
	}
};

class Cell::TextImpl : public Impl {
public:
	TextImpl(std::string_view text)
	{
		text_ = text;
		value_ = std::string(text);
		if (text[0] == ESCAPE_SIGN)
		{
			value_ = std::string(text.substr(1));
		}
	}

	Value GetValue() const
	{
		return value_;
	}

	std::string GetText() const
	{
		return text_;
	}

};

class Cell::FormulaImpl : public Impl {
public:
	FormulaImpl(std::string_view expr, SheetInterface& sheet) : sheet_(sheet)
	{
		if (expr[0] == FORMULA_SIGN)
		{
			expr = expr.substr(1);
			value_ = std::string(expr);
			formula_ptr_ = ParseFormula(std::move(std::string(expr)));
			text_ = FORMULA_SIGN + formula_ptr_->GetExpression();
		}
	}

	Value GetValue() const
	{
		if (IIsCached())
		{
			return cache_.value();
		}
		auto result = formula_ptr_->Evaluate(sheet_);
		if (std::holds_alternative<double>(result))
		{
			cache_ = std::get<double>(result);
			return std::get<double>(result);
		}
		else
		{
			return std::get<FormulaError>(result);
		}
		
	}

	std::string GetText() const
	{
		return text_;
	}

	bool IIsCached() const override
	{
		if (cache_.has_value())
		{
			return true;
		}
		return false;
	}

	void IInvalidateCache() override
	{
		cache_.reset();
	}

	std::vector<Position> IGetReferencedCells() const override
	{
		return formula_ptr_->GetReferencedCells();
	}

private:
	std::unique_ptr<FormulaInterface> formula_ptr_;
	mutable std::optional<CellInterface::Value> cache_;
    SheetInterface& sheet_;

};


Cell::Cell(Sheet& sh) :sheet_(sh), impl_(std::make_unique<EmptyImpl>())
{}


Cell::~Cell() = default;

void Cell::Set(std::string text)
{
	std::unique_ptr<Impl> new_impl;
	if (text.size() == 0)
	{
		new_impl = std::make_unique<EmptyImpl>();
	}
	else if (text.size() > 1 && text[0] == FORMULA_SIGN)
	{
		new_impl = std::make_unique<FormulaImpl>(std::move(text), sheet_);
	}
	else
	{
		new_impl = std::make_unique<TextImpl>(std::move(text));
	}
	if (IsCyclicDependent(*new_impl))
	{
		throw CircularDependencyException("Circ");
	}

	impl_ = std::move(new_impl);

	for (Cell* right : right_nodes_)
	{
		right->left_nodes_.erase(this);
	}

	right_nodes_.clear();

	for (const Position& pos : impl_->IGetReferencedCells())
	{
		Cell* right = static_cast<Cell*>(sheet_.GetCell(pos));
		if (!right)
		{
			sheet_.SetCell(pos, "");			
			right = static_cast<Cell*>(sheet_.GetCell(pos));
		}
		right_nodes_.insert(right);
		right->left_nodes_.insert(this);
	}
	InvalidateCell();
}

void Cell::Clear()
{
	impl_ = std::make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const
{
	return impl_->GetValue();
}
std::string Cell::GetText() const
{
	return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const
{
	return impl_->IGetReferencedCells();
}

void Cell::InvalidateCache()
{
	impl_->IInvalidateCache();
}

void Cell::InvalidateCell()
{
	if (impl_->IIsCached())
	{
		impl_->IInvalidateCache();
	}
	for (const auto& cell : left_nodes_)
	{
		cell->InvalidateCell();
	}
}

bool Cell::IsCached() const
{
	return impl_->IIsCached();
}

bool Cell::IsCyclicDependent(const Impl& new_impl) const
{
	if (new_impl.IGetReferencedCells().empty())
	{
		return false;
	}
	std::unordered_set<const Cell*> referenced;

	for(const auto& pos : new_impl.IGetReferencedCells())
	{
		referenced.insert(static_cast<const Cell*>(sheet_.GetCell(pos)));
	}
	std::unordered_set<const Cell*> visited;
	std::vector<const Cell*> to_visit;
	to_visit.push_back(this);
	while (!to_visit.empty())
	{
		const Cell* current = to_visit.back();
		to_visit.pop_back();
		visited.insert(current);

		if (referenced.find(current) != referenced.end())
		{
			return true;
		}

		for (const Cell* left : current->left_nodes_)
		{
			if (visited.find(left) == visited.end())
			{
				to_visit.push_back(left);
			}
		}
	}

	return false;
}

bool Cell::IsReferenced() const
{
	return !left_nodes_.empty();
}


