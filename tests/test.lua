
local function main(xlsx_file)
  local proc = io.popen('./xlsx2lua --no-pretty --skip-empty-row '..xlsx_file)
  local stdout = proc:read('*a')
  proc:close()

  local book = load(stdout)()
  print('book: '..xlsx_file)
  for _, sheet in ipairs(book.sheets) do
    print(string.rep('-', 80))
    print('sheet.name: '..sheet.name)
    local cells = sheet.cells
    for y, row in ipairs(cells) do
      for x, col in ipairs(row) do
        print(string.format('cell(%d,%d): %s', x, y, col))
      end
    end
  end
end

main("tests/sample.xlsx")
