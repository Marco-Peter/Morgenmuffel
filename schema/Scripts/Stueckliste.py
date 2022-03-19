#!/usr/bin/python
import sys, io
import xml.etree.ElementTree as ET

if len(sys.argv) < 2:
	print "Input file is missing!"

if len(sys.argv) < 3:
	print "Output file is missing!"
	sys.exit(-1)

inputFile = sys.argv[1]
if sys.argv[2].split('.')[-1] != "csv":
	outputFile = sys.argv[2] + ".csv"
else:
	outputFile = sys.argv[2]

tree = ET.parse(inputFile)
root = tree.getroot()

columns = set()
comps = {}

if root.find('components') is not None:
	# Parse over all components
	for inComp in root.find('components').findall('comp'):
		if (inComp.find('footprint').text, inComp.find('value').text) in comps.keys():
			comp = comps[(inComp.find('footprint').text, inComp.find('value').text)]
			comp['refs'].add(inComp.attrib['ref'])
		else:
			comp = {}
			comp['refs'] = set([inComp.attrib['ref']])
			comps[(inComp.find('footprint').text, inComp.find('value').text)] = comp
		
		if inComp.find('fields') is not None:
			for field in inComp.find('fields').findall('field'):
				comp[field.attrib['name']] = field.text
				columns.add(field.attrib['name'])

columns = sorted(columns)

with io.open(outputFile, 'w', encoding='utf8') as f:
	f.write(u'item;footprint;value;count;refs')
	for col in columns:
		f.write(u';')
		f.write(unicode(col))
	f.write(u'\n')
	
	for comp, item in zip(comps, range(1, len(comps) + 1)):
		f.write(unicode(item))
		f.write(u';')
		f.write(unicode(comp[0]))
		f.write(u';')
		f.write(unicode(comp[1]))
		f.write(u';')
		f.write(unicode(len(comps[comp]['refs'])))
		f.write(u';')
		for ref in sorted(comps[comp]['refs']):
			f.write(unicode(ref))
			if ref != sorted(comps[comp]['refs'])[-1]:
				f.write(u', ')
		f.write(u';')
		
		for col in columns:
			try:
				f.write(unicode(comps[comp][col]))
			except KeyError:
				f.write(u'--')
			f.write(u';')
		f.write(u'\n')

