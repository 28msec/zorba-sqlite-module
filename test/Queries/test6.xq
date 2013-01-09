import module namespace s = "http://www.zorba-xquery.com/modules/sqlite";

let $db := s:connect("small2.db")

return {
  variable $comm := s:commit($db);
  variable $roll := s:rollback($db);
    
  $comm = $roll
}
