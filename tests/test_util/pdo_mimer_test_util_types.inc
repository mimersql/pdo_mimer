<?php

/**
 * All the Mimer data types
 */
enum PDOMimerDataType {
    case CHAR; case VARCHAR; case CLOB;
    case NCHAR; case NVARCHAR; case NCLOB;
    case BINARY; case VARBINARY; case BLOB;
    case SMALLINT; case INT; case BIGINT;
    case DEC; case REAL;
    case DOUBLE_PRECISION; case FLOAT;
    case DATE; case TIME; case TIMESTAMP;
    case INTERVAL;
    case BOOLEAN;
}

/**
 *  Maps the Mimer data types to the corresponding 
 *  PDO data type constant and PHP type name. 
 */
class PDOMimerDataTypeMap {
    public static array $map;

    public static function getMimerTypeName(PDOMimerDataType $type): string {
        foreach(self::$map as $entry)
            if($entry[0] === $type)
                return $entry[1];
    }

    public static function getPDOParamConst(PDOMimerDataType $type): ?int {
        foreach(self::$map as $entry)
            if($entry[0] === $type)
                return $entry[2];
    }

    public static function getPHPTypeName(PDOMimerDataType $type): array {
        foreach(self::$map as $entry)
            if($entry[0] === $type)
                return $entry[3];
        return "";
    }
}

/**
 * Format:
 * Enumeration type | Name in Mimer | PDO Parameter type | Compatible PHP types
 * 
 * Note: When integers get big enough, PHP stores them as doubles. 
 * This is e.g. why BIGINT must have several compatible types.
 * 
 * Note: LOBs can be used with both _STR and _LOB constants, depending on
 * if user wants to deal with them as file pointers or strings. Currently 
 * they're default mapping is to _STR. Usage of _LOB is done explicitely 
 * in certain tests.
 */
PDOMimerDataTypeMap::$map = [
    [PDOMimerDataType::CHAR, "CHAR", PDO::PARAM_STR, ["string"]],
    [PDOMimerDataType::VARCHAR, "VARCHAR", PDO::PARAM_STR, ["string"]], 
    [PDOMimerDataType::CLOB, "CLOB", PDO::PARAM_STR, ["string"]],
    [PDOMimerDataType::NCHAR, "NCHAR", PDO::PARAM_STR, ["string"]],
    [PDOMimerDataType::NVARCHAR, "NVARCHAR", PDO::PARAM_STR, ["string"]],
    [PDOMimerDataType::NCLOB, "NCLOB", PDO::PARAM_STR, ["string"]],
    [PDOMimerDataType::BINARY, "BINARY", PDO::PARAM_STR, ["string"]],
    [PDOMimerDataType::VARBINARY, "VARBINARY", PDO::PARAM_STR, ["string"]],
    [PDOMimerDataType::BLOB, "BLOB", PDO::PARAM_STR, ["string"]],
    [PDOMimerDataType::SMALLINT, "SMALLINT", PDO::PARAM_INT, ["integer"]],
    [PDOMimerDataType::INT, "INT", PDO::PARAM_INT, ["integer"]],
    [PDOMimerDataType::BIGINT, "BIGINT", PDO::PARAM_INT, ["integer", "double"]],
    [PDOMimerDataType::DEC, "DEC", PDO::PARAM_STR, ["string"]],
    [PDOMimerDataType::REAL, "REAL", PDO::PARAM_STR, ["string"]],
    [PDOMimerDataType::DOUBLE_PRECISION, "DOUBLE PRECISION", PDO::PARAM_STR, ["string"]],
    [PDOMimerDataType::FLOAT, "FLOAT", PDO::PARAM_STR, ["string"]],
    [PDOMimerDataType::DATE, "DATE", PDO::PARAM_STR, ["string"]],
    [PDOMimerDataType::TIME, "TIME", PDO::PARAM_STR, ["string"]],
    [PDOMimerDataType::TIMESTAMP, "TIMESTAMP", PDO::PARAM_STR, ["string"]],
    [PDOMimerDataType::INTERVAL, "INTERVAL HOUR", PDO::PARAM_STR, ["string"]], // Only supports hour-intervals for now
    [PDOMimerDataType::BOOLEAN, "BOOLEAN", PDO::PARAM_BOOL, ["boolean"]],
];



