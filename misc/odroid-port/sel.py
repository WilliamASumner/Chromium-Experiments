import collections
import textwrap

from selenium import webdriver
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support.expected_conditions import staleness_of
from xvfbwrapper import Xvfb

import sys
import json
import datetime

'''
addressArray = [
		'file:///home/odroid/bbench-3.0/sites/amazon/www.amazon.com/index.html',
		'file:///home/odroid/bbench-3.0/sites/bbc/www.bbc.co.uk/index.html', 
		'file:///home/odroid/bbench-3.0/sites/cnn/www.cnn.com/index.html', 
		'file:///home/odroid/bbench-3.0/sites/craigslist/newyork.craigslist.org/index.html', 
		'file:///home/odroid/bbench-3.0/sites/ebay/www.ebay.com/index.html', 
		'file:///home/odroid/bbench-3.0/sites/espn/espn.go.com/index.html', 
		'file:///home/odroid/bbench-3.0/sites/google/www.google.com/index.html', 
		'file:///home/odroid/bbench-3.0/sites/msn/www.msn.com/index.html', 
		'file:///home/odroid/bbench-3.0/sites/slashdot/slashdot.org/index.html', 
		'file:///home/odroid/bbench-3.0/sites/twitter/twitter.com/index.html', 
		'file:///home/odroid/bbench-3.0/sites/youtube/www.youtube.com/index.html']
'''

addressArray = [
		'http://tucunare.cs.pitt.edu:8080/amazon/www.amazon.com/',
		'http://tucunare.cs.pitt.edu:8080/bbc/www.bbc.co.uk/', 
		'http://tucunare.cs.pitt.edu:8080/cnn/www.cnn.com/', 
		'http://tucunare.cs.pitt.edu:8080/craigslist/newyork.craigslist.org/', 
		'http://tucunare.cs.pitt.edu:8080/ebay/www.ebay.com/',
		'http://tucunare.cs.pitt.edu:8080/google/www.google.com/', 
		'http://tucunare.cs.pitt.edu:8080/msn/www.msn.com/', 
		'http://tucunare.cs.pitt.edu:8080/slashdot/slashdot.org/', 
		'http://tucunare.cs.pitt.edu:8080/twitter/twitter.com/', 
		'http://tucunare.cs.pitt.edu:8080/youtube/www.youtube.com/'
		]

sites = [
	'amazon',
	'bbc', 
	'cnn',
	'craigslist', 
	'ebay',
	'google',
	'msn', 
	'slashdot',
	'twitter', 
	'youtube'
	]

class PageLoadTimer:
	def __init__(self,driver):
		self.driver = driver
		self.jscript = textwrap.dedent("""
		    var performance = window.performance || {};
			var timings = performance.timing || {};
			return timings;
			""")
	def inject_timing_js(self):
		timings = self.driver.execute_script(self.jscript)
		return timings
	def get_event_times(self):
		timings = self.inject_timing_js()
		good_values = [epoch for epoch in timings.values() if epoch != 0] # filter out bad timestamps

		ordered_events = ('startTimestamp','navigationStart','fetchStart',
						  'domainLookupStart', 'domainLookupEnd','connectStart',
						  'connectEnd', 'secureConnectionStart','requestStart',
						  'responseStart','responseEnd','domLoading',
						  'domInteractive','domContentLoadedEventStart',
						  'domContentLoadedEventEnd', 'domComplete',
						  'loadEventStart','loadEventEnd' )
		# have one entry from the start of the execution and one from the actual timestamp
		startTimestamp = min(good_values)
		timings['startTimestamp'] = startTimestamp
		event_times = ((event, (timings[event],timings[event] - startTimestamp if timings[event] - startTimestamp >= 0 else 0)) for event 
		                in ordered_events if event in timings)
		return collections.OrderedDict(event_times)

def saveResults(iters,sitesused,timestamps):
	with open('output.json','w') as jsonFile:
		info = {"iterations":iters, "sites":sitesused, "timestamps":timestamps}
		content = json.dumps(info)
		jsonFile.write(content)

def runTest(sitesused,iters,addresses):
	results = dict(zip(sitesused,[[] for site in sitesused])) # empty results dict
	with Xvfb() as xvfb:
		options = webdriver.ChromeOptions()
		options.add_argument('--ignore-certificate-errors')
		options.add_argument("--test-type")
		options.binary_location = "/usr/bin/chromium-browser"
		driver = webdriver.Chrome(chrome_options=options)
		averagingCoeff = 1.0/iters
		for i in range(iters):
			for index,address in enumerate(addresses):
				print("loading address: " + address)
				driver.get(address)
				timer = PageLoadTimer(driver)
				results[sitesused[index]].append(timer.get_event_times())
	return results


if __name__ == '__main__':
	iterations = 400 # just do one iteration for now, we'll do more in the main shell script
	site_num = int(sys.argv[1])
	siteused = [ sites[site_num] ]
	addressused = [ addressArray[site_num] ]
	saveResults(iterations,siteused,runTest(siteused,iterations,addressused))
