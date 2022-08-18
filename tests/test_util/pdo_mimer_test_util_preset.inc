<?php
/**
 * Represents a database preset.  
 */
class PDOMimerTestPreset {
    private string $name;           // name of DB preset
    private array $tables = [];     // assoc. array of tables in preset

    function __construct(string $name) {
        $this->name = $name;
    }

    public function addTable(PDOMimerTestTable $table){
        $this->tables[$table->getName()] = $table;
    }

    public function getName(){
        return $this->name; 
    }

    public function getTables(): array {
        return $this->tables;
    }

    public function getTable(string $tblName): ?PDOMimerTestTable {
        foreach ($this->tables as $table)
            if($table->getName() === $tblName)
                return $table;
    }

}
?>