#include <algorithm>

#include <allegro5/allegro_native_dialog.h>
#include "Save.h"
#include "gameplay.h"

#include "drawing.h"
#include "functions.h"
#include "load.h"
#include "misc_structs.h"
#include "utils/string_utils.h"
#include "vars.h"


void savemobs() {
	data_node master("masternode", "");
	data_node* mobs_node = new data_node("mobs", "");
	data_node* onions_node = new data_node("onions", "");
	int id = 0;
	int lig = 0;

	for (size_t m = 0; m < mobs.size(); ++m) {
		mob* m_ptr = mobs[m];
		int temp = m_ptr->groupid;
		if (lig < temp) {
			lig = temp;
		}
	}
	vector<int> group_amounts;
	size_t n_groups = lig;
	for (size_t s = 0; s < n_groups + 1; ++s) {
		group_amounts.push_back(0);
	}
	for (size_t m = 0; m < mobs.size(); ++m) {
		mob* m_ptr = mobs[m];
		int temp = m_ptr->groupid;
		if (group_amounts[temp+1] != 0) {
			group_amounts[temp+1] += 1;

		}
		else {
			group_amounts[temp+1] = 1;
		}

		
	}
	vector<bool>groups;

	int groupsmissing = 0;
	for (size_t s = 0; s < group_amounts.size(); ++s) {
		if (group_amounts[s] == 0) {
			groupsmissing += 1;

			for (size_t m = 0; m < mobs.size(); ++m) {
				mob* m_ptr = mobs[m];
				if (m_ptr->groupid > s - groupsmissing - 1) {
					m_ptr->groupid += -1;

				}
			}
		}
		else { 
			mobs_node->add(new data_node("mobgroupV" + i2s(s - groupsmissing - 1), "")); 
		}
	}
	data_node* groupAmount = new data_node("ga", i2s(lig));
	for (size_t m = 0; m < mobs.size(); ++m) {
		mob* m_ptr = mobs[m];
		data_node* mobsl_node = mobs_node->get_child(m_ptr->groupid +1);
		data_node* mob_node =
			new data_node(m_ptr->type->category->name, "");
		mobsl_node->add(mob_node);
		
		if (m_ptr->type->category->name == "Onion") {
				onion* o = (onion*)m_ptr;
				data_node* o_node = new data_node(o->type->name, "");
				onions_node->add(o_node);
					o_node->add(new data_node("Leaf_Pikmin_Inside",
						i2s(o->pikmin_inside[0])));
					o_node->add(new data_node("Bud_Pikmin_Inside",
						i2s(o->pikmin_inside[1])));
					o_node->add(new data_node("Flower_Pikmin_Inside",
						i2s(o->pikmin_inside[2])));
					o_node->add(new data_node("Fourth_Maturity",
						i2s(o->pikmin_inside[4])));
			}
		else if (m_ptr->type) {
			mob_node->add(
				new data_node("type", m_ptr->type->name)
			);
		}
		mob_node->add(
			new data_node(
				"p",
				f2s(m_ptr->pos.x) + " " + f2s(m_ptr->pos.y)
			)
		);
		if (m_ptr->angle != 0) {
			mob_node->add(
				new data_node("angle", f2s(m_ptr->angle))
			);
		}
		if (m_ptr->vars.size()) {
			string sing = "vars=";
			vector<string> names = m_ptr->varnames;
			for (size_t v = 0; v < m_ptr->varnames.size(); ++v) {
				string bing = names[v];
				string wing = m_ptr->vars.at(bing);
				sing += bing + "=" + wing;
				sing += " ";
			}
			mob_node->add(new data_node(sing, ""));
		}
		if (m_ptr->lid != -1)
			mob_node->add(
				new data_node("group", i2s(m_ptr->groupid))
			);
	}

	master.add(mobs_node);
	master.add(groupAmount);
	master.add(onions_node);
	string filessavename = AREAS_FOLDER_PATH + "/" + cur_area_data.name +
		"/Mobs_on_Day" + i2s(day) + ".txt";
	bool mob_save_ok = master.save_file(filessavename);
}