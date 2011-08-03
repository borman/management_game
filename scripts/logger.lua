#!/usr/bin/env lua

-- Game logger script
-- Connects to the server and prints game log in gnuplot-suitable format to stdout

-- Configuration
host = "localhost"
port = 8982

-- Dependencies
local socket = require "socket"
local lpeg = require "lpeg"
local re = require "re"

function make_tokenizer()
  local space = lpeg.P' '
  local quote = lpeg.P'"'
  local literal = lpeg.C((1-(space+quote))^1)
  local quoted = quote * lpeg.C((1-quote)^1) * quote
  local token = literal + quoted
  return lpeg.Ct(token * (space^1 * token)^0)
end

local tokenizer = make_tokenizer()

function tokenize(str)
  return lpeg.match(tokenizer, str)
end

local c, err = socket.connect(host, port)
if c == nil then -- Error
  print("Error connecting to server - "..err)
  os.exit(1)
end

function send(str)
  c:send(str.."\n")
end

function connection()
  while true do -- Main loop
    local rl, sl, err = socket.select({c}, nil, 3)
    if err ~= nil then
      print("Error in select - "..err)
      break
    end
    if rl[1] ~= nil then -- Data available
      while true do
        -- Read a line
        local line, err = c:receive()
        if line ~= nil then
          coroutine.yield(tokenize(line))
        else
          break
        end
      end
    end
  end
  coroutine.yield(nil)
end
getline_raw = coroutine.wrap(connection)


-- Auth
while true do
  local line = getline_raw()
  if line[1] == "$" and line[2] == "auth" then
    send("auth super logger")
    while true do
      local line = getline_raw()
      if line[1] == "ack" then
        break
      elseif line[1] == "fail" then
        print("auth failed:", line[2])
        os.exit(1)
      end
    end
    break
  end
end

function match(a, b)
  if #a < #b then
    return false
  end
  for i=1,#b do
    if a[i] ~= b[i] then
      return false
    end
  end
  return true
end

-- Main
round_counter = 0
players = {}
while true do
  local line = getline_raw()
  if match (line, {"+", "game", "start"}) then
    round = 0
    players = {}
  elseif match (line, {"+", "player"}) then
    players[line[3]] = line[4]
  elseif match (line, {"+", "bankrupt"}) then
    players[line[3]] = 0
  elseif match (line, {"+", "round", "start"}) then
    local round = tonumber(line[4])

    if round == 0 then
      local line = "Round "
      for k,v in pairs(players) do
        line = line .. string.format("%10s ", '"'..k..'"')
      end
      print(line)
    end

    local line = string.format("%5d ", round)
    for k,v in pairs(players) do
      line = line .. string.format("%10d ", v)
    end
    print(line)
  elseif match (line, {"+", "game", "end"}) then
    break
  end
end

