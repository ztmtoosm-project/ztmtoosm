#include "readztm.hpp"
#include "osm_base.hpp"
#include "starelinie.hpp"

struct wspolrzedne
{
	double lon;
	double lat;
};


struct HafasPrzejazd;
struct HafasLinia;

struct HafasStop
{
	string id;
	string name;
	wspolrzedne wspol;
	string miejscowosc;
	map <HafasStop*, vector <HafasPrzejazd*> > wychodzace;
	map <HafasStop*, vector <HafasPrzejazd*> > wchodzace;
};

struct HafasPrzejazd
{
	HafasLinia* linia;
	HafasPrzejazd* poprzedni;
	HafasPrzejazd* nastepny;
	int timestart;
	int timestop;
	HafasStop* pierwszy;
	HafasStop* drugi;
};

struct HafasLinia
{
	string id;
	vector <HafasPrzejazd*> kursy;
	vector <vector <string> > trasy;
};

struct DijData
{
	set <HafasStop*> odwiedzone;
	set <pair<int, HafasStop*> > kandydaci1;
	map <HafasStop*, int> kandydaci2;
	map <HafasStop*, HafasPrzejazd*> poprzednik;
	void update(HafasStop* node, HafasPrzejazd* pop, int time, bool reverse)
	{
		if(odwiedzone.find(node)!=odwiedzone.end())
			return;
		if(kandydaci2.find(node)!=kandydaci2.end())
		{
			int oldtime=kandydaci2[node];
			if((oldtime>time && !reverse) || (oldtime<time && reverse))
			{
				kandydaci1.erase(kandydaci1.find(pair<int, HafasStop*>(oldtime, node)));
				kandydaci2[node]=time;
				poprzednik[node]=pop;
				kandydaci1.insert(pair<int, HafasStop*>(time, node));
			}
		}
		else
		{
				kandydaci2[node]=time;
				poprzednik[node]=pop;
				kandydaci1.insert(pair<int, HafasStop*>(time, node));
		}
	}
	void update(HafasStop* node, HafasPrzejazd* pop, int time)
	{
		update(node,pop,time, false);
	}
	int wstaw(HafasStop* node, int time)
	{
		kandydaci1.insert(pair<int, HafasStop*>(time, node));
		kandydaci2[node]=time;
	}
	int odwiedz(HafasStop* node, int ewstart)
	{
		int oldtime=ewstart;
		if(kandydaci2.find(node)!=kandydaci2.end())
		{
			oldtime=kandydaci2[node];
			if(kandydaci1.find(pair<int, HafasStop*>(oldtime, node))!=kandydaci1.end())
				kandydaci1.erase(kandydaci1.find(pair<int, HafasStop*>(oldtime, node)));
			kandydaci2.erase(node);
		}
		odwiedzone.insert(node);
		return oldtime;
	}
	vector <HafasPrzejazd*> poprzednicy(HafasStop* start)
	{
		vector <HafasPrzejazd*> wynik;
		while(poprzednik.find(start)!=poprzednik.end())
		{
			wynik.push_back(poprzednik[start]);
			if(poprzednik[start]->pierwszy!=start)
				start=poprzednik[start]->pierwszy;
			else
				start=poprzednik[start]->drugi;
		}
		reverse(wynik.begin(), wynik.end());
		return wynik;
	}
};


class HafasBazaLoader;

class HafasBaza
{
	friend class HafasBazaLoader;
	public:
	HafasBaza() {};
	map <string, HafasLinia*> linie;
	map <string, HafasStop*> przystanki;
	map <pair <string, string>, vector <wspolrzedne> > laczniki;
	bool podobneid(string alfa, string beta)
	{
		if(alfa.substr(0, 4)==beta.substr(0, 4))
		{
			return true;
		}
		return false;
	}
	void wypelnij_sciezki(map<string, HafasStop*>::iterator it1)
	{
		map<string, HafasStop*>::iterator it2=it1;
		bool okminus=1;
		bool okplus=1;
		if(it2==przystanki.begin())
			okminus=0;
		it2--;
		while(okminus)
		{
			if(podobneid(it1->second->id, it2->second->id))
			{
				HafasPrzejazd* nowy = new HafasPrzejazd;
				nowy->linia=NULL;
				nowy->poprzedni=NULL;
				nowy->nastepny=NULL;
				nowy->timestart=0;
				nowy->timestop=120;
				nowy->pierwszy=it1->second;
				nowy->drugi=it2->second;
				it1->second->wychodzace[it2->second].push_back(nowy);
				it2->second->wchodzace[it1->second].push_back(nowy);
			}
			else
			{
				okminus=0;
			}
			if(it2==przystanki.begin())
				okminus=0;
			it2--;
		}
		it2=it1;
		it2++;
		while(okplus)
		{
			if(it2==przystanki.end())
				okplus=0;
			if(okplus)
			{
				if(podobneid(it1->second->id, it2->second->id))
				{
					HafasPrzejazd* nowy = new HafasPrzejazd;
					nowy->linia=NULL;
					nowy->poprzedni=NULL;
					nowy->nastepny=NULL;
					nowy->timestart=0;
					nowy->timestop=120;
					nowy->pierwszy=it1->second;
					nowy->drugi=it2->second;
					it1->second->wychodzace[it2->second].push_back(nowy);
					it2->second->wchodzace[it1->second].push_back(nowy);
				}
				else
				{
					okplus=0;
				}
				it2++;
			}
		}
	}
	void wypelnij_sciezki()
	{
		auto it1=przystanki.begin();
		while(it1!=przystanki.end())
		{
			wypelnij_sciezki(it1);
			it1++;
		}
	}
	/*void sortRecordy()
	{
		auto it1=przystanki.begin();
		while(it1!=przystanki.end())
		{
			sort((it1->second).recordy.begin(), (it1->second).recordy.end(), srt);
			it1++;
		}
	}
	int binsearch(vector <OneStopRecord> recordy, int time)
	{
		int left=0;
		int right=recordy.size()-1;
		while(right-left>2)
		{
			if(recordy[(left+right)/2].time<time)
			{
				left=(left+right)/2;
			}
			else
			{
				right=(left+right)/2;
			}
		}
		return left;
	}*/
	vector <HafasPrzejazd*> dijkstra (int time, HafasStop* start, HafasStop* stop)
	{
		DijData dix;
		dix.wstaw(start, time);
		while(!dix.kandydaci1.empty() && dix.odwiedzone.find(stop)==dix.odwiedzone.end())
		{
			HafasStop* akt=dix.kandydaci1.begin()->second;
			int akttime=dix.odwiedz(akt, time);
			map <HafasStop*, vector <HafasPrzejazd*> > recordy = akt->wychodzace;
			HafasPrzejazd* poprzednik=NULL;
			if(dix.poprzednik.find(akt)!=dix.poprzednik.end())
			{
				poprzednik=dix.poprzednik[akt];
			}
			auto it1=recordy.begin();
			while(it1!=recordy.end())
			{
				vector <HafasPrzejazd*> rec2 = it1->second;
				for(int i=0; i<rec2.size(); i++)
				{
					if(rec2[i]->linia==NULL)
					{
						dix.update(rec2[i]->drugi, rec2[i], akttime+rec2[i]->timestop);
					}
					else if(rec2[i]->timestart>=akttime)
					{
						if(poprzednik==NULL || (poprzednik->linia==rec2[i]->linia) || poprzednik->linia==NULL)
						{
							dix.update(rec2[i]->drugi, rec2[i], rec2[i]->timestop);
						}
						else
						{
							if(rec2[i]->timestart>akttime)
							{
								dix.update(rec2[i]->drugi, rec2[i], rec2[i]->timestop);
							}
						}
					}
				}
				it1++;
			}
		}
		return dix.poprzednicy(stop);
	}
	vector <HafasPrzejazd*> dijkstraReverse (int time, HafasStop* start, HafasStop* stop)
	{
		DijData dix;
		dix.wstaw(stop, time);
		cout<<"aaa"<<endl;
		while(!dix.kandydaci1.empty() && dix.odwiedzone.find(start)==dix.odwiedzone.end())
		{
			cout<<"bbb"<<endl;
			auto it_pom=dix.kandydaci1.end();
			it_pom--;
			HafasStop* akt=(it_pom)->second;
			cout<<akt<<endl;
			cout<<"aktid "<<akt->id<<endl;
			int akttime=dix.odwiedz(akt, time);
			map <HafasStop*, vector <HafasPrzejazd*> > recordy = akt->wchodzace;
			HafasPrzejazd* poprzednik=NULL;
			if(dix.poprzednik.find(akt)!=dix.poprzednik.end())
			{
				poprzednik=dix.poprzednik[akt];
			}
			auto it1=recordy.begin();
			while(it1!=recordy.end())
			{
				vector <HafasPrzejazd*> rec2 = it1->second;
				for(int i=0; i<rec2.size(); i++)
				{
					if(rec2[i]->linia==NULL)
					{
						dix.update(rec2[i]->pierwszy, rec2[i], akttime-rec2[i]->timestop, true);
					}
					else if(rec2[i]->timestop<=akttime)
					{
						if(poprzednik==NULL || (poprzednik->linia==rec2[i]->linia) || poprzednik->linia==NULL)
						{
							dix.update(rec2[i]->pierwszy, rec2[i], rec2[i]->timestart, true);
						}
						else
						{
							if(rec2[i]->timestop<akttime)
							{
								dix.update(rec2[i]->pierwszy, rec2[i], rec2[i]->timestart, true);
							}
						}
					}
				}
				it1++;
			}
		}
		auto wynik = dix.poprzednicy(start);
		cout<<wynik.size()<<endl;
		reverse(wynik.begin(), wynik.end());
		return wynik;
	}
	void dijkstra(string start, string stop, int time, ostream& strim)
	{
		cout<<"dupa0"<<endl;
		vector <HafasPrzejazd*> dll=dijkstraReverse(time, przystanki[start], przystanki[stop]);
		cout<<"dupa1"<<endl;
		pair <HafasPrzejazd*, HafasPrzejazd*> akt = pair<HafasPrzejazd*, HafasPrzejazd*>(NULL, NULL);
		vector <pair<HafasPrzejazd*, HafasPrzejazd*> > kursy;
		for(int i=0; i<dll.size(); i++)
		{
			if(dll[i]->linia!=NULL)
			{
				if(akt.first==NULL || dll[i]->linia!=akt.second->linia)
				{
					if(akt.first!=NULL)
					{
						kursy.push_back(akt);
					}
					akt.first=dll[i];
				}
				akt.second=dll[i];
			}
		}
		kursy.push_back(akt);
		wypiszKurs(kursy, strim);
	}
	void wypiszPartRoute(int starttime, int stoptime, string start, string stop, ostream& strim, bool ostatni)
	{
		vector <wspolrzedne> obrobka;
		if(laczniki.find(pair<string, string>(start, stop))!=laczniki.end())
		{
			obrobka=laczniki[pair<string, string>(start, stop)];
		}
		else
		{
			obrobka.push_back(przystanki[start]->wspol);
			obrobka.push_back(przystanki[stop]->wspol);
		}
		for(int i=0; i<obrobka.size(); i++)
		{
			if(ostatni || i<obrobka.size()-1)
			{
				string type="point";
				strim<<"{";
				strim<<"\"lat\":"<<obrobka[i].lat;
				strim<<", "<<"\"lon\":"<<obrobka[i].lon;
				if(i==0)
				{
					type="stop";
					strim<<", "<<"\"name\":\""<<avoid_cudzyslow(przystanki[start]->name)<<"\"";
					strim<<", "<<"\"id\":\""<<przystanki[start]->id<<"\"";
					if(starttime>0)
						strim<<", "<<"\"time\":"<<starttime;
				}
				if(i==obrobka.size()-1)
				{
					type="stop";
					strim<<", "<<"\"name\":\""<<avoid_cudzyslow(przystanki[stop]->name)<<"\"";
					strim<<", "<<"\"id\":\""<<przystanki[stop]->id<<"\"";
					if(stoptime>0)
						strim<<", "<<"\"time\":"<<stoptime;
				}
				strim<<", "<<"\"type\":\""<<type<<"\"";
				strim<<"}";
				if(i<obrobka.size()-1)
					strim<<",";
			}
		}
	}
	void wypiszRoute(string id, ostream& strim)
	{
		strim<<"[";
		for(int i=0; i<linie[id]->trasy.size(); i++)
		{
			strim<<"[";
			for(int j=0; j<linie[id]->trasy[i].size()-1; j++)
			{
				bool ostatni=(j==linie[id]->trasy[i].size()-2);
				wypiszPartRoute(-1, -1, linie[id]->trasy[i][j], linie[id]->trasy[i][j+1], strim, ostatni);
			}
			strim<<"]";
			if(i<linie[id]->trasy.size()-1)
				strim<<",";
		}
		strim<<"]";
	}
	void wypiszKurs (vector <pair<HafasPrzejazd*, HafasPrzejazd*> >kursy, ostream& strim)
	{
		strim<<"[";
		for(int i=0; i<kursy.size(); i++)
		{
			strim<<"{";
			strim<<"\"line\": \""<<kursy[i].first->linia->id<<"\",";
			strim<<"\"route\": ";
			strim<<"[";
			bool ostatni=0;
			HafasPrzejazd* akt=kursy[i].first;
			while(!ostatni && akt!=NULL)
			{
				if(akt==kursy[i].second)
					ostatni=1;
				wypiszPartRoute(akt->timestart, akt->timestop, akt->pierwszy->id, akt->drugi->id, strim, ostatni);
				akt=akt->nastepny;
			}
			strim<<"]";
			strim<<"}";
			if(i<kursy.size()-1)
				strim<<",";
		}
		strim<<"]";
	}
	/*
	void wypiszRoute(vector <string> id, ostream& strim)
	{
		strim<<"[";
		for(int g=0; g<id.size(); g++)
		{
			for(int i=0; i<linie[id[g]].trasy.size(); i++)
			{
				strim<<"[";
				for(int j=0; j<linie[id[g]].trasy[i].size()-1; j++)
				{
					bool ostatni=(j==linie[id[g]].trasy[i].size()-2);
					wypiszPartRoute(-1, -1,linie[id[g]].trasy[i][j], linie[id[g]].trasy[i][j+1], strim, ostatni);
				}
				strim<<"]";
				if(i<linie[id[g]].trasy.size()-1 || g<id.size()-1)
					strim<<",";
			}
		}
		strim<<"]";
	}*/
};

class HafasBazaLoader : ztmread
{
	HafasBaza* baza;
	private:
	void nowy_kurs(kurs nowy)
	{
		vector <HafasPrzejazd*> dodane;
		for(int i=0; i<nowy.postoje.size()-1; i++)
		{
			HafasPrzejazd* nowy2 = new HafasPrzejazd;
			nowy2->linia=baza->linie[nowy.linia];
			nowy2->poprzedni=NULL;
			if(dodane.size()>0)
			{
				nowy2->poprzedni=dodane[dodane.size()-1];
				dodane[dodane.size()-1]->nastepny=nowy2;
			}
			nowy2->nastepny=NULL;
			nowy2->timestart=nowy.postoje[i].time;
			nowy2->timestop=nowy.postoje[i+1].time;
			nowy2->pierwszy=baza->przystanki[nowy.postoje[i].stop_id];
			nowy2->drugi=baza->przystanki[nowy.postoje[i+1].stop_id];
			baza->przystanki[nowy.postoje[i].stop_id]->wychodzace[baza->przystanki[nowy.postoje[i+1].stop_id]].push_back(nowy2);
			baza->przystanki[nowy.postoje[i+1].stop_id]->wchodzace[baza->przystanki[nowy.postoje[i].stop_id]].push_back(nowy2);
			dodane.push_back(nowy2);
		}
		if(dodane.size()>0)
		{
			baza->linie[nowy.linia]->kursy.push_back(dodane[0]);
		}
	}
	void nowy_przystanek(przystanek nowy)
	{
		HafasStop* nowy2 = new HafasStop;
		nowy2->name = nowy.name;
		nowy2->id = nowy.id;
		nowy2->miejscowosc = nowy.miejscowosc;
		nowy2->wspol.lon = nowy.lon;
		nowy2->wspol.lat = nowy.lat;
		baza->przystanki[nowy.id] = nowy2;
	}
	void nowa_linia(string nazwa, vector <vector <string> > trasy)
	{
		baza->linie[nazwa]=new HafasLinia;
		baza->linie[nazwa]->id=nazwa;
		baza->linie[nazwa]->trasy=trasy;
	}
	public:
	HafasBazaLoader (HafasBaza* baz, string sciezka) : baza(baz), ztmread(sciezka)
	{
		run();
	}
};
class OsmBazaLoader
{
	vector <long long> bfs(long long start, long long stop, map <long long, vector <long long> >& prepareBfs)
	{
		vector <long long> wynik;
		vector <long long> kolejka;
		int poz=0;
		kolejka.push_back(start);
		set <long long> odwiedzone;
		odwiedzone.insert(start);
		map <long long, long long> ojc;
		bool ok=1;
		while(poz!=kolejka.size() && ok)
		{
			vector <long long> nachte=prepareBfs[kolejka[poz]];
			for(int i=0; i<nachte.size(); i++)
			{
				if(odwiedzone.find(nachte[i])==odwiedzone.end())
				{
					kolejka.push_back(nachte[i]);
					ojc[nachte[i]]=kolejka[poz];
					odwiedzone.insert(nachte[i]);
					if(nachte[i]==stop)
						ok=0;
				}
			}
			poz++;
		}
		long long akt=stop;
		while(ojc.find(akt)!=ojc.end())
		{
			wynik.push_back(akt);
			akt=ojc[akt];
		}
		wynik.push_back(start);
		reverse(wynik.begin(), wynik.end());
		return wynik;
	}
	void easyNodes(long long rel, osm_base* base, HafasBaza *base2)
	{
		relation akt_rel = base->relations[rel];
		map <long long, vector <long long> > prepareBfs;
		map <string, long long> refs;
		for(int i=0; i<akt_rel.members.size(); i++)
		{
			if(akt_rel.members[i].member_type==WAY)
			{
				long long way_id=akt_rel.members[i].member_id;
				if(akt_rel.members[i].role!="platform")
				if(akt_rel.members[i].role!="platform_entry_only")
				if(akt_rel.members[i].role!="platform_exit_only")
				{
					vector <long long> nods=base->ways[way_id].nodes;
					for(int j=0; j<nods.size()-1; j++)
					{
						prepareBfs[nods[j]].push_back(nods[j+1]);
						prepareBfs[nods[j+1]].push_back(nods[j]);
					}
					for(int j=0; j<nods.size(); j++)
					{
						if(base->nodes[nods[j]].tags["public_transport"]=="stop_position")
						{
							refs[base->nodes[nods[j]].tags["ref"]]=nods[j];
						}
					}
				}
			}
		}
		vector <string> ref1;
		for(int i=0; i<akt_rel.members.size(); i++)
		{
			if(akt_rel.members[i].member_type==NODE)
			{
				bool ok=0;
				long long node_id=akt_rel.members[i].member_id;
				if(akt_rel.members[i].role=="stop")
					ok=1;
				if(akt_rel.members[i].role=="stop_entry_only")
					ok=1;
				if(akt_rel.members[i].role=="stop_exit_only")
					ok=1;
				if(ok && base->nodes[node_id].tags["ref"]!="")
				{
					string ref=base->nodes[node_id].tags["ref"];
					ref1.push_back(ref);
				}
			}
		}
		for(int i=0; i<ref1.size()-1; i++)
		{
			vector <long long> bfstmp=bfs(refs[ref1[i]], refs[ref1[i+1]], prepareBfs);
			vector <wspolrzedne> wynik;
			for(int j=0; j<bfstmp.size(); j++)
			{
				wspolrzedne foo;
				foo.lat=base->nodes[bfstmp[j]].lat;
				foo.lon=base->nodes[bfstmp[j]].lon;
				wynik.push_back(foo);
			}
			base2->laczniki[pair<string, string>(ref1[i], ref1[i+1])]=wynik;
		}
	}
	void easyNodes(string linia, osm_base* base, HafasBaza *base2)
	{
		vector <long long> relacje=relacje_linia(base, 3651336, linia).second;
		vector <vector <long long> > wynik;
		for(int i=0; i<relacje.size(); i++)
		{
			easyNodes(relacje[i], base, base2);
		}
	}
	public:
	OsmBazaLoader(HafasBaza* baz, string sciezka)
	{
		osm_base bazaOsm(sciezka);
		auto it1=bazaOsm.nodes.begin();
		while(it1!=bazaOsm.nodes.end())
		{
			if((it1->second).tags["public_transport"]=="stop_position")
			{
				string ref=(it1->second).tags["ref"];
				if(baz->przystanki.find(ref)!=baz->przystanki.end())
				{
					wspolrzedne wsp;
					wsp.lat=(it1->second).lat;
					wsp.lon=(it1->second).lon;
					baz->przystanki[ref]->wspol=wsp;
					baz->przystanki[ref]->name=(it1->second).tags["name"];
				}
			}
			it1++;
		}
		auto it2=baz->linie.begin();
		while(it2!=baz->linie.end())
		{
			easyNodes(it2->first, &bazaOsm, baz);
			it2++;
		}
		baz->wypelnij_sciezki();
	}
};