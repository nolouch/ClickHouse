#include <DB/DataStreams/SquashingTransform.h>


namespace DB
{

SquashingTransform::SquashingTransform(size_t min_block_size_rows, size_t min_block_size_bytes)
	: min_block_size_rows(min_block_size_rows), min_block_size_bytes(min_block_size_bytes)
{
}


SquashingTransform::Result SquashingTransform::add(Block && block)
{
	if (!block)
		return Result(std::move(accumulated_block));

	/// Just read block is alredy enough.
	if (isEnoughSize(block.rowsInFirstColumn(), block.bytes()))
	{
		/// If no accumulated data, return just read block.
		if (!accumulated_block)
			return Result(std::move(block));

		/// Return accumulated data (may be it has small size) and place new block to accumulated data.
		accumulated_block.swap(block);
		return Result(std::move(block));
	}

	/// Accumulated block is already enough.
	if (accumulated_block && isEnoughSize(accumulated_block.rowsInFirstColumn(), accumulated_block.bytes()))
	{
		/// Return accumulated data and place new block to accumulated data.
		accumulated_block.swap(block);
		return Result(std::move(block));
	}

	append(std::move(block));

	if (isEnoughSize(accumulated_block.rowsInFirstColumn(), accumulated_block.bytes()))
	{
		Block res;
		res.swap(accumulated_block);
		return Result(std::move(res));
	}

	/// Squashed block is not ready.
	return false;
}


void SquashingTransform::append(Block && block)
{
	if (!accumulated_block)
	{
		accumulated_block = std::move(block);
		return;
	}

	size_t columns = block.columns();
	size_t rows = block.rowsInFirstColumn();

	for (size_t i = 0; i < columns; ++i)
		accumulated_block.unsafeGetByPosition(i).column->insertRangeFrom(
			*block.unsafeGetByPosition(i).column, 0, rows);
}


bool SquashingTransform::isEnoughSize(size_t rows, size_t bytes) const
{
	return (!min_block_size_rows && !min_block_size_bytes)
		|| (min_block_size_rows && rows >= min_block_size_rows)
		|| (min_block_size_bytes && bytes >= min_block_size_bytes);
}

}
