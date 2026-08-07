import gdb
import re


# Pretty printer for a Pikifen Mob class.
class MobPrinter:
    def __init__(self, val):
        self.val = val
        self.fields = self.val.type.fields()

    def children(self):
        yield "(Mob type)", self.get_mob_type_name()
        for field in self.fields:
            if field.name.startswith("_vptr"):
                continue
            yield field.name, self.val[field]

    def get_mob_type_name(self):
        result = "Null"
        mob_type_child = self.val["type"]
        mob_type = None

        if mob_type_child:
            mob_type = mob_type_child.dereference()

        if mob_type:
            result = mob_type["name"]

        return result


# Registers the printer if the type matches Mob.
def mob_lookup(val):
    CLASS_NAME = "Mob"

    typ = val.type.strip_typedefs()

    if typ.code == gdb.TYPE_CODE_PTR:
        target = typ.target().strip_typedefs()
        if target.code == gdb.TYPE_CODE_STRUCT and target.tag == CLASS_NAME:
            return MobPrinter(val.dereference())

    if typ.code == gdb.TYPE_CODE_STRUCT and typ.tag == CLASS_NAME:
        return MobPrinter(val)

    return None


# Register the printer.
gdb.pretty_printers.append(mob_lookup)
