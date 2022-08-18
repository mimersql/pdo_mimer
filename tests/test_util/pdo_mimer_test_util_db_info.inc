<?php
/**
 * Responsible for allowing tester to ask questions 
 * about the state of the database, such as e.g. 
 * table/ident existence.
 */
class PDOMimerTestDBInfo {

    public static function objectExists(string $objectType, string $objectId): bool {
        $selectFormat = "SELECT COUNT(*) FROM INFORMATION_SCHEMA.%s WHERE %s = '%s'";
        switch (strtoupper($objectType)) {
            case "TABLE":
                $viewName = "TABLES";
                $idColumn = "TABLE_NAME";
                break;
            case "IDENT":
                $viewName = "EXT_IDENTS";
                $idColumn = "IDENT_NAME";
                break;
            default:
                die("objectExists() does not yet support checking type $objectType\n");
        }
        $select = sprintf($selectFormat, $viewName, $idColumn, $objectId);

        $db = new PDO(PDOMimerTestConfig::getDSN());
        $stmt = $db->query($select);
        $count = $stmt->fetch(PDO::FETCH_NUM)[0];
        return $count !== 0;
    }
}


?>