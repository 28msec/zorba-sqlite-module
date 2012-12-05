import module namespace s = "http://www.zorba-xquery.com/modules/sqlite";

let $db := s:connect("small2.db")
let $comm := s:commit($db)
let $roll := s:rollback($db)
let $db := s:disconnect($db)
    
return ( $comm = $roll, $comm = $db )
