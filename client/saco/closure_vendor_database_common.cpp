#include "../raknet/TableSerializer.h"
#include "../raknet/LightweightDatabaseCommon.h"
#include "../raknet/BitStream.h"
#include "../raknet/StringCompressor.h"

void DatabaseFilter::Serialize(RakNet::BitStream *out)
{
	stringCompressor->EncodeString(columnName, _TABLE_MAX_COLUMN_NAME_LENGTH, out);
	out->Write((unsigned char)columnType);
	out->Write((unsigned char)operation);
	if (operation!=DataStructures::Table::QF_IS_EMPTY && operation!=DataStructures::Table::QF_NOT_EMPTY)
	{
		assert(cellValue.isEmpty==false);
		TableSerializer::SerializeCell(out, &cellValue, columnType);
	}
}
bool DatabaseFilter::Deserialize(RakNet::BitStream *in)
{
	unsigned char temp;
	stringCompressor->DecodeString(columnName, _TABLE_MAX_COLUMN_NAME_LENGTH, in);
	in->Read(temp);
	columnType=(DataStructures::Table::ColumnType)temp;
	if (in->Read(temp)==false)
		return false;
	operation=(DataStructures::Table::FilterQueryType)temp;
	if (operation!=DataStructures::Table::QF_IS_EMPTY && operation!=DataStructures::Table::QF_NOT_EMPTY)
	{
		return TableSerializer::DeserializeCell(in, &cellValue, columnType);
	}
	return true;
}
void DatabaseCellUpdate::Serialize(RakNet::BitStream *out)
{
	stringCompressor->EncodeString(columnName, _TABLE_MAX_COLUMN_NAME_LENGTH, out);
	out->Write((unsigned char)columnType);
	TableSerializer::SerializeCell(out, &cellValue, columnType);
}
bool DatabaseCellUpdate::Deserialize(RakNet::BitStream *in)
{
	unsigned char temp;
	stringCompressor->DecodeString(columnName, _TABLE_MAX_COLUMN_NAME_LENGTH, in);
	in->Read(temp);
	columnType=(DataStructures::Table::ColumnType)temp;
	return TableSerializer::DeserializeCell(in, &cellValue, columnType);
}

typedef char R5TableCellSize[(sizeof(DataStructures::Table::Cell)==9)?1:-1];
typedef char R5TableRowSize[(sizeof(DataStructures::Table::Row)==12)?1:-1];
typedef char R5TableColumnSize[(sizeof(DataStructures::Table::ColumnDescriptor)==36)?1:-1];
typedef char R5TableSize[(sizeof(DataStructures::Table)==36)?1:-1];
typedef char R5DatabaseFilterSize[(sizeof(DatabaseFilter)==49)?1:-1];
typedef char R5DatabaseCellUpdateSize[(sizeof(DatabaseCellUpdate)==45)?1:-1];
