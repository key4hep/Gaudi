import win32com.client
import gaudiextra

class Excel:
	def __init__(self):
		self.ex = win32com.client.Dispatch('Excel.Application')
		self.wb = self.ex.Workbooks.Add()
		self.ws = self.wb.WorkSheets.Add()
		self.ws.Name = 'HistoData'
		self.ch = self.wb.Charts.Add()
		self.ch.Name = 'HistoPlot'
	def plot(self, h):
		self.ex.Visible = 0
		heights = h.heights()
		nbin = len(heights)
		xmin, xmax = h.edges()
		for i in range(nbin) :
			self.ws.Cells(i+1,1).value = heights[i]
			self.ws.Cells(i+1,2).value = xmin + i*(xmax-xmin)/nbin 
		self.ch.SetSourceData(self.ws.Range(self.ws.Cells(1,1), self.ws.Cells(nbin+1,1)))
		self.ch.HasTitle = 1
		self.ch.ChartTitle.Text = h.title()
		self.ser = self.ch.SeriesCollection(1)
		self.ser.XValues = self.ws.Range(self.ws.Cells(1,2), self.ws.Cells(nbin+1,2))
		self.ex.Visible = 1

global excel
excel = Excel()



