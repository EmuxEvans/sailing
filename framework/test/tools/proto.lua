print("start");
obj = new_my_struct();
obj.a_int = 9090;

print("obj.a_int = "..obj.a_int);
proto_save(obj, "obj.txt");
proto_load(obj, "obj.txt");
print("obj.a_int = "..obj.a_int);

print("end");
