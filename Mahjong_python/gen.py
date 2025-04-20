import copy
import argparse
import xml.etree.ElementTree as ET
import os

class GameRecord:
	def __init__(self):
		self.record = [None for i in range(4)]
		self.outrecord = []
		self.total = []
		
	def write_out(self, filename):
		with open(filename, mode="w", encoding="utf-8") as f:
			for r in self.total:
				f.write("{0}\n".format(r))
				
	def read_one(self, child):
		#U E 1
		#V F 2
		#W G 3
		#T D 0
		fmu = child.tag[0:1]
		#print(child.tag, fmu)
		if child.tag == 'GO':
			return
		if child.tag == 'UN':
			return
		if child.tag == 'TAIKYOKU':
			return
		if child.tag == 'DORA':
			return
		if child.tag == 'INIT':
			#self.total.append(self.record)
			self.record = [None for i in range(4)]
			self.record[0] = self.get_card_from_string(child.attrib['hai0'])
			self.record[1] = self.get_card_from_string(child.attrib['hai1'])
			self.record[2] = self.get_card_from_string(child.attrib['hai2'])
			self.record[3] = self.get_card_from_string(child.attrib['hai3'])
			
		if fmu == 'U':
			c = child.tag[1:len(child.tag)]
			#print("玩家1 摸牌:{0}".format(c))
			self.in_card(self.record[1], int(c))
			
		if fmu == 'E':
			c = child.tag[1:len(child.tag)]
			#print("玩家1 出牌:{0} {1}".format(c, self.get_out_string(self.record[1], int(c))))
			self.total.append(self.get_out_string(self.record[1], int(c)))
			self.out_card(self.record[1], int(c))
			
		if fmu == 'V':
			c = child.tag[1:len(child.tag)]
			#print("玩家2 摸牌:{0}".format(c))
			self.in_card(self.record[2], int(c))
			
		if fmu == 'F':
			c = child.tag[1:len(child.tag)]
			#print("玩家2 出牌:{0} {1}".format(c, self.get_out_string(self.record[2], int(c))))
			self.total.append(self.get_out_string(self.record[2], int(c)))
			self.out_card(self.record[2], int(c))
			
		if fmu == 'W':
			c = child.tag[1:len(child.tag)]
			#print("玩家3 摸牌:{0}".format(c))
			self.in_card(self.record[3], int(c))
			
		if fmu == 'G':
			c = child.tag[1:len(child.tag)]
			#print("玩家3 出牌:{0} {1}".format(c, self.get_out_string(self.record[3], int(c))))
			self.total.append(self.get_out_string(self.record[3], int(c)))
			self.out_card(self.record[3], int(c))

		if fmu == 'T':
			c = child.tag[1:len(child.tag)]
			#print("玩家0 摸牌:{0}".format(c))
			self.in_card(self.record[0], int(c))
			
		if fmu == 'D':
			c = child.tag[1:len(child.tag)]
			#print("玩家0 出牌:{0} {1}".format(c, self.get_out_string(self.record[0], int(c))))
			self.total.append(self.get_out_string(self.record[0], int(c)))
			self.out_card(self.record[0], int(c))

		if child.tag == 'N':
			#print("玩家 鸣牌")
			who = int(child.attrib['who'])
			other = int(child.attrib['m']) & 3
			ischi = (int(child.attrib['m']) >> 2) & 1
			ispen = (int(child.attrib['m']) >> 3) & 1
			isgang = (int(child.attrib['m']) >> 3) & 3
			if ischi == 1:
				out = (int(child.attrib['m']) >> 10) & 63
				_k = int(out / 3)
				_h = int(out % 3)
				_v = int(_k % 7)
				_c = int(_k / 7)
				_ot = _c * 36 + _v * 4 + _h * 4
				_ot1 = _c * 36 + _v * 4 + 0
				_ot2 = _c * 36 + _v * 4 + 1 * 4
				_ot3 = _c * 36 + _v * 4 + 2 * 4
				_ex1 = (int(child.attrib['m']) >> 3) & 3
				_ex2 = (int(child.attrib['m']) >> 5) & 3
				_ex3 = (int(child.attrib['m']) >> 7) & 3
				#print("{0}吃:{1} {2} {3} {4}".format(who, _ot1, _ot2, _ot3, _ot))
				if _ot1 == _ot:
					self.out_card(self.record[who], _ot2+_ex2)
					self.out_card(self.record[who], _ot3+_ex3)
					#print("{0}吃:{1} {2}".format(who, _ot2+_ex2, _ot3+_ex3))
				if _ot2 == _ot:
					self.out_card(self.record[who], _ot1+_ex1)
					self.out_card(self.record[who], _ot3+_ex3)
					#print("{0}吃:{1} {2}".format(who, _ot1+_ex1, _ot3+_ex3))
				if _ot3 == _ot:
					self.out_card(self.record[who], _ot1+_ex1)
					self.out_card(self.record[who], _ot2+_ex2)
					#print("{0}吃:{1} {2}".format(who, _ot1+_ex1, _ot2+_ex2))
			elif ispen == 1:
				out = (int(child.attrib['m']) >> 9) & 127
				_k = int(out / 3)
				_h = int(out % 3)
				_ot = _k * 4
				_tp = 0
				_unuse = (int(child.attrib['m']) >> 5) & 3
				#print("{0}碰:{1} {2} {3}".format(who, _ot, _h, _unuse))
				if _unuse != 0:
					if _h != _tp:
						self.out_card(self.record[who], _ot+0)
					_tp = _tp + 1
				if _unuse != 1:
					if _h != _tp:
						self.out_card(self.record[who], _ot+1)
					_tp = _tp + 1
				if _unuse != 2:
					if _h != _tp:
						self.out_card(self.record[who], _ot+2)
					_tp = _tp + 1
				if _unuse != 3:
					if _h != _tp:
						self.out_card(self.record[who], _ot+3)
					_tp = _tp + 1
			elif ispen == 0:
				if isgang == 0:
					isjiagang = (int(child.attrib['m']) >> 4) & 1
					if other == 0:
						out = (int(child.attrib['m']) >> 8) & 255
						#print("{0}暗杠: {1} {2}".format(who, out, child.attrib))
						
						_ex = int(out % 4)
						self.out_card(self.record[who], out - _ex)
						self.out_card(self.record[who], out - _ex + 1)
						self.out_card(self.record[who], out - _ex + 2)
						self.out_card(self.record[who], out - _ex + 3)
					elif isjiagang == 1:
						#print("{0}加杠:".format(who))
						out = (int(child.attrib['m']) >> 9) & 127
						_ex = (int(child.attrib['m']) >> 5) & 3
						_ot = int(out / 3) * 4 + _ex
						self.out_card(self.record[who], _ot)
					elif isjiagang == 0:
						#print("{0}明杠:".format(who))
						out = (int(child.attrib['m']) >> 8) & 255
						_ex = out % 4
						_base = out - _ex
						if _ex != 0:
							self.out_card(self.record[who], _base + 0)
						if _ex != 1:
							self.out_card(self.record[who], _base + 1)
						if _ex != 2:
							self.out_card(self.record[who], _base + 2)
						if _ex != 3:
							self.out_card(self.record[who], _base + 3)
	def get_card_from_string(self, str):
		tmp = str.split(',')
		for index in range(len(tmp)):
			tmp[index] = int(tmp[index])
		return tmp
		
	def get_out_string(self, r, c):
		_str = ""
		for index in range(len(r)):
			_str = _str + self.get_hai_string(r[index])
		for index in range(len(r), 14):
			_str = _str + '0'
		_str = _str + " " + self.get_hai_string(c)
		return _str
	def get_hai_string(self, num):
		_out = num - (num % 4)
		_i = int(_out / 36)
		_z = int((_out % 36) / 4)
		_out = _i * 9 + _z
		#print(num, _out)
		table = ["1m", "2m", "3m", "4m", "5m", "6m", "7m", "8m", "9m",
				 "1p", "2p", "3p", "4p", "5p", "6p", "7p", "8p", "9p",
				 "1s", "2s", "3s", "4s", "5s", "6s", "7s", "8s", "9s",
				 "東", "南", "西", "北", "白", "発", "中"]
		return table[_out]
		
	def out_card(self, r, c):
		_old = len(r)
		for index in range(len(r)):
			if r[index] == c:
				del r[index]
				break
		_after = len(r)
		if _old == _after:
			raise RuntimeError('err{0} {1} {2}'.format(_old, c, r))
		return r
		
	def in_card(self, r, c):
		r.append(c)
		return r

if __name__ == '__main__':
	arg_parser = argparse.ArgumentParser(prog = "mahjong_generator", add_help = True)
	arg_parser.add_argument("-i", "--input", help = "input file", default="./xml/")
	arg_parser.add_argument("-o", "--output", help = "output file", default="dahai_data.txt")
	arg_parser.add_argument("-l", "--limit", help = "limit of number of games", type=int, default=40000)
	args = arg_parser.parse_args()

	game_record = GameRecord()
	#game_record.read_record(args.input, args.limit)
	#game_record.write_to_file(args.output)
	_ct = 0
	for root, dirs, files in os.walk(args.input):
		print("total: {0}".format(len(files)))
		for path in files:
			_ct = _ct + 1
			print("Cur: {0}/{1}".format(_ct,len(files)))
			try:
				tree = ET.parse("./xml/" + path)
				root = tree.getroot()
				root_lists = []
				for child in root:
					#print(child.tag, child.attrib)
					game_record.read_one(child)
			except:
				print("COntinue")
	game_record.write_out(args.output)
