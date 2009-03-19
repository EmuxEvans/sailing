
function onconnect(conn)
	output("--OnConnect ("..conn..") --\n")
end

function ondisconnect(conn)
	output("--OnDisconnect ("..conn..") --\n")
end

function serialize(o, userdata)
	if userdata=="" then
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
		end
	end
	
	if type(o) == userdata then
		output("{");
		output("name="..o.name..", ");
		output("guild="..o.guild..",");
		output("}");
	end
end

function ondata(conn, args)
	output("--OnData ("..conn..") CmdCode("..args.CmdCode..") CmdName("..args.CmdName..")--\n");
	local count = 0;
	for k,v in pairs(args) do
		if k~="CmdName" and k~="CmdCode" then
			if type(v)~="userdata" then
				count = count + 1;
				output("  "..k.." = ")
				serialize(v, "")
				output(",")
			end
		end
	end
	if count>0 then
		output("\n");
	end
	for k,v in pairs(args) do
		if k~="CmdName" and k~="CmdCode" then
			if type(v)=="userdata" then
				output("  "..k.." = ")
				serialize(v, "userdata")
				output(",\n")
			end
		end
	end
end
