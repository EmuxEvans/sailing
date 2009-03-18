
function onconnect()
	output("--OnConnect--\n")
end

function ondisconnect()
	output("--OnDisconnect--\n")
end

function serialize (o)
	if type(o) == "number" then
		output(o)
	elseif type(o) == "string" then
		output(string.format("%q", o))
	elseif type(o) == "table" then
		output("{")
		for k,v in pairs(o) do
			output(" ["..k.."] = ")
			serialize(v)
			output(",")
		end
		output("}")
	else
		output("cannot serialize a ")
	end
end

function ondata(args)
	output("--OnData CmdName(" .. args.CmdName..") CmdCode("..args.CmdCode..")--\n");
	for k,v in pairs(args) do
		if k~="CmdName" and k~="CmdCode" then
			output("  "..k.." = ")
			serialize(v)
			output(",\n")
		end
	end
end
