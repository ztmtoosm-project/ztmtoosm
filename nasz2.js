var palette = ['#ab0d15'
,'#F7931D'
/*,'#FFF200'*/
,'#0DB14B'
,'#0071BC'
,'#00B9F1'
,'#903F98'
,'#EE609D'];
function getpalette(poz, n)
{
	console.log(Math.floor(poz*7/n));
	return palette[Math.floor(poz*7/n)];
}
function getpaletteid(poz, n)
{
	return Math.floor(poz*7/n);
}

function loadJSON(url) 
{
	var resp;
	var xmlHttp;
	resp  = '';
	xmlHttp = new XMLHttpRequest();

	if(xmlHttp != null)
	{
		xmlHttp.open( "GET", url, false );
		xmlHttp.send( null );
		resp = xmlHttp.responseText;
	}
	return JSON.parse(resp); 
}
function podrozSciezka (from, to, time)
{
	var sciezka="http://127.0.0.1/hafas";
	sciezka+='\?from\='+from;
	sciezka+='\&to\='+to;
	sciezka+='\&time\='+time;
	return sciezka;
}	

function addLineToSource (coordinates, colorLine, name, vectorSource1, vectorSource2)
{
	var warstwa1 = new ol.Feature({geometry: new ol.geom.LineString(coordinates)});
	var warstwaStroke1 = new ol.style.Stroke ({color: colorLine, width: 4});
	var warstwaStrokeText1 = new ol.style.Stroke ({color: '#fff', width: 6});
	var warstwaFont1 = '18px helvetica,sans-serif,bold';
	var warstwaFill1 = new ol.style.Fill({color: colorLine});
	var warstwaText1 = new ol.style.Text({text: name, font: warstwaFont1, fill: warstwaFill1, stroke: warstwaStrokeText1});
	var warstwaStyle1 =new ol.style.Style({stroke : warstwaStroke1, text : warstwaText1});
	warstwa1.setStyle(warstwaStyle1);
	var warstwa2 = new ol.Feature({geometry: new ol.geom.LineString(coordinates)});
	var warstwaStroke2 = new ol.style.Stroke ({color: '#fff', width: 7});
	var warstwaStyle2 = new ol.style.Style({stroke : warstwaStroke2});
	warstwa2.setStyle(warstwaStyle2);
	vectorSource1.addFeature(warstwa1);
	vectorSource2.addFeature(warstwa2);
}
var PrzystanekNaMapie = function (name, id, coordinates, vectorSource)
{
	this.name = name;
	this.coordinates = coordinates;
	this.propsedColor = null;
	this.id = id;
	this.minTime = null;
	this.minTimeLine = null;
	this.maxTime = null;
	this.maxTimeLine = null;
	this.feature = new ol.Feature({geometry: new ol.geom.Point(coordinates)});
	this.feature.setId(id);
	vectorSource.addFeature(this.feature);
};

PrzystanekNaMapie.prototype.addLine = function (time, line, color)
{
	if(this.propsedColor == null)
		this.propsedColor = color;
	else
		this.propsedColor = '#000';
	if(this.minTime = null || time < this.mintime)
	{
		this.minTime = time;
		this.minTimeLine = line;
	}
	if(this.maxTime = null || time < this.maxtime)
	{
		this.maxTime = time;
		this.maxTimeLine = line;
	}
};

PrzystanekNaMapie.prototype.setBasicStyle = function ()
{
	this.feature.setStyle(
		new ol.style.Style({
	    image: new ol.style.Circle({
	      fill: new ol.style.Fill({
		color: this.propsedColor
	      }),
	      radius: 8,
	      stroke: new ol.style.Stroke({
		color: '#fff',
		width: 2
	      })
	    })
	  }));
};

PrzystanekNaMapie.prototype.setExtraStyle = function ()
{
	var name = this.name;
	var warstwaStrokeText1 = new ol.style.Stroke ({color: '#fff', width: 6});
	var warstwaFont1 = '18px helvetica,sans-serif,bold';
	var warstwaFill1 = new ol.style.Fill({color: this.propsedColor});
	var txt = new ol.style.Text({text: name, font: warstwaFont1, fill: warstwaFill1, stroke: warstwaStrokeText1});
	this.feature.setStyle(
		new ol.style.Style({
	    image: new ol.style.Circle({
	      fill: new ol.style.Fill({
		color: this.propsedColor
	      }),
	      radius: 12,
	      stroke: new ol.style.Stroke({
		color: '#fff',
		width: 4
	      })
	    }), text : txt
	  }));

};

var WizualizacjaPodrozy = function (from, to, time)
{
	this.bazaJSON = loadJSON(podrozSciezka(from, to, time));
	this.heart=document.getElementById("heart");
	heart.innerHTML="";
	this.warstwaVector1 = new ol.source.Vector({});
	this.warstwaVector2 = new ol.source.Vector({});
	var podstawa=loadJSON(podrozSciezka(from, to, time));
	this.przystanki = [];
	for(var i=0; i < this.bazaJSON.length; i++)
	{
		var nowalinia=document.createElement("DIV");
		var nowaliniaid=document.createElement("DIV");
		nowaliniaid.innerHTML=podstawa[i].line;
		nowaliniaid.className="hafasliniaid";
		nowalinia.appendChild(nowaliniaid);
		nowalinia.className="hafaslinia";
		nowalinia.setAttribute("paletteid", getpaletteid(i, podstawa.length));
		heart.appendChild(nowalinia);
		var tablica1 = [];
		for(var j=0; j < this.bazaJSON[i].route.length; j++)
		{
			var tmp=ol.proj.transform([podstawa[i].route[j].lon, podstawa[i].route[j].lat], 'EPSG:4326', 'EPSG:3857');
			tablica1[tablica1.length] = tmp;
			if(podstawa[i].route[j].type=="stop")
			{
				if(this.przystanki[podstawa[i].route[j].id]==undefined)
				{
					this.przystanki[podstawa[i].route[j].id] = new PrzystanekNaMapie (podstawa[i].route[j].name, podstawa[i].route[j].id, tmp, this.warstwaVector1);
				}
				var tim=podstawa[i].route[j].time;
				this.przystanki[podstawa[i].route[j].id].addLine(tim, podstawa[i].line, getpalette(i, podstawa.length));
				var nowystop=document.createElement("DIV");
				var nowystopname=document.createElement("DIV");
				var nowystoptime=document.createElement("DIV");
				nowystopname.innerHTML=podstawa[i].route[j].name;
				tim/=60;
				var mins=tim%60;
				var hours=Math.floor(tim/60);
				nowystoptime.innerHTML=hours+":"+mins;
				nowystop.appendChild(nowystopname);
				nowystop.appendChild(nowystoptime);
				nowystop.className="hafasstop";
				nowalinia.appendChild(nowystop);
			}
		}
		addLineToSource(tablica1, getpalette(i, this.bazaJSON.length), 'dupa', this.warstwaVector1, this.warstwaVector2);
		for (var item in this.przystanki)
		{
			this.przystanki[item].setBasicStyle();
		}
		/*var fea_tmp3 = new ol.Feature({geometry: new ol.geom.MultiPoint(tablica2)});
		fea_tmp3.setStyle(
		new ol.style.Style({
	    image: new ol.style.Circle({
	      fill: new ol.style.Fill({
		color: getpalette(i, podstawa.length)
	      }),
	      radius: 8,
	      stroke: new ol.style.Stroke({
		color: '#fff',
		width: 2
	      })
	    
	    })
	  }));
		this.warstwaVector1.addFeature(fea_tmp3);
	*/
	}
};


/*
var style = {
  'MultiPoint': [new ol.style.Style({
    image: new ol.style.Circle({
      fill: new ol.style.Fill({
        color: 'rgba(255,255,0,255)',
      radius: 12,
      stroke: new ol.style.Stroke({
        color: '#ff0',
        width: 1
      })
    })
  })],
  'MultiLineString': [new ol.style.Style({
    stroke: new ol.style.Stroke({
      color: '#f0f',
      width: 12
    })
  })],
  'LineString': [new ol.style.Style({
    stroke: new ol.style.Stroke({
      color: '#fff',
      width: 5
    })
  })]
};
var style2 = {
  'MultiPoint': [new ol.style.Style({
    image: new ol.style.Circle({
      fill: new ol.style.Fill({
        color: 'rgba(255,255,0,255)'
      }),
      radius: 12,
      stroke: new ol.style.Stroke({
        color: '#ff0',
        width: 1
      })
    })
  })],
  'MultiLineString': [new ol.style.Style({
    stroke: new ol.style.Stroke({
      color: '#000',
      width: 1
    })
  })],
  'LineString': [new ol.style.Style({
    stroke: new ol.style.Stroke({
      color: '#f07',
      width: 2
    })
  })]
};
*/
function loadJSON(url) {
var resp ;
var xmlHttp ;

resp  = '' ;
xmlHttp = new XMLHttpRequest();

if(xmlHttp != null)
{
xmlHttp.open( "GET", url, false );
xmlHttp.send( null );
resp = xmlHttp.responseText;
}
       return JSON.parse(resp); 
}

var tabsource = null;

var basicVector = new ol.source.Vector({});
var projection = ol.proj.get('EPSG:3857');
var layerLines = new ol.layer.Vector({
source: basicVector,
    style: function(feature, resolution) {
    return style[feature.getGeometry().getType()];
  }
});
var layerLines2 = new ol.layer.Vector({
  source: basicVector,
    style: function(feature, resolution) {
    return style2[feature.getGeometry().getType()];
  }
});
      var map = new ol.Map({
        target: 'map',
        layers: [
          new ol.layer.Tile({
            source: new ol.source.OSM({url: "http://a.tiles.wmflabs.org/bw-mapnik/{z}/{x}/{y}.png", crossOrigin: null})
          }), layerLines, layerLines2],
        view: new ol.View({
          center: ol.proj.transform([37.41, 8.82], 'EPSG:4326', 'EPSG:3857'),
          zoom: 4
        })});

    	document.getElementById("from").value="701301";
    	document.getElementById("to").value="122201";
    	document.getElementById("time").value="36000";
	var aktfeature = null;
    	var mapcnt=document.getElementById("map");
	
mapcnt.onmousemove = function(e)
{
	var pos   = {X : mapcnt.offsetLeft, Y : mapcnt.offsetTop};
	var mPos  = {X : e.clientX - pos.X, Y : e.clientY - pos.Y};
	var newfeature = null;
	map.forEachFeatureAtPixel([mPos.X, mPos.Y], function(feature, layer){
	if(feature.getGeometry().getType()=='Point')
	{
		newfeature=feature.getId();
	}
	});
	if(newfeature!=aktfeature)
	{
		if(newfeature!=null)
		{
			tabsource.przystanki[newfeature].setExtraStyle();
		}
		if(aktfeature!=null)
		{
			tabsource.przystanki[aktfeature].setBasicStyle();
		}
		aktfeature=newfeature;
	}
}

function changesource()
{
    	var from = document.getElementById("from").value;
    	var to = document.getElementById("to").value;
    	var time = document.getElementById("time").value;
	map.removeLayer(layerLines2);
	map.removeLayer(layerLines);
	tabsource = new WizualizacjaPodrozy(from, to, time);
	layerLines = new ol.layer.Vector({
	source: tabsource.warstwaVector2,
	style: function(feature, resolution)
	{
		return style[feature.getGeometry().getType()];
	}
	});
	layerLines2 = new ol.layer.Vector({
	source: tabsource.warstwaVector1,
	style: function(feature, resolution) {
	return style2[feature.getGeometry().getType()];
	}
	});
 	map.addLayer(layerLines);
        map.addLayer(layerLines2);
}
