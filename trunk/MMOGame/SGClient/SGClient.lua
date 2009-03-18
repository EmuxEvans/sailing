
function onconnect()
	output("XXOnConnect")
end

function ondisconnect()
	output("XXOnDisconnect")
end

function ondata(args)
	output("XXOnData " .. args.CmdName)
	if args.CmdName=="login_seed" then
	end;
end
