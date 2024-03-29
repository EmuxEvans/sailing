
function onconnect(conn)
	output("--OnConnect ("..conn..") --\n")
end

function ondisconnect(conn)
	output("--OnDisconnect ("..conn..") --\n")
end

function serialize(o)
	if type(o) == "number" then
		output(o)
	elseif type(o) == "string" then
		output(string.format("%q", o))
	elseif type(o) == "table" then
		output("{\n")
		for k,v in pairs(o) do
			if type(k) == "number" then
				output(" ["..k.."] = ")
			else
				output(" "..k.." = ")
			end
			serialize(v)
			output(",")
		end
		output("}")
	end
end

function ondata(conn, args)
	output("--OnData ("..conn..") CmdCode("..args.CmdCode..") CmdName("..args.CmdName..")--\n");
	local count = 0;
	for k,v in pairs(args) do
		if k~="CmdName" and k~="CmdCode" then
			count = count + 1;
			output("  "..k.." = ")
			serialize(v)
			output(",")
		end
	end
	output("\n")
end
