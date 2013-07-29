import module namespace s = "http://zorba.io/modules/sqlite";

let $db := s:connect("small2.db")

return {
  variable $comm := s:commit($db);
  variable $roll := s:rollback($db);
    
  $comm = $roll
}
