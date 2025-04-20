# -*- coding:utf-8 -*-
import sys
import io
import os
import argparse
import time
import cv2
from pyzbar import pyzbar
# import tensorflow as tf
import mahjong_common as mjc
import mahjong_loader as mjl
import tensorflow as tf 


import re   #引入正则表达式对象  
import urllib   #用于对URL进行编解码  
from http.server import HTTPServer, BaseHTTPRequestHandler
import json
import process_ai
import serial_comm


tf.compat.v1.disable_eager_execution()

NUM_HAI = 34
NUM_HIDDEN_LAYERS = 3
H_SIZE = 200

sess = None

x = None
y = None
train_labels = None
train_predictions = None
test_labels = None
test_predictions = None

train_step = None
out = None
train_accuracy = None
train_update_op = None
test_accuracy = None
test_update_op = None

def make_model():
    global x
    global y
    global train_labels
    global train_predictions
    global test_labels
    global test_predictions

    global train_step
    global out
    global train_accuracy
    global train_update_op
    global test_accuracy
    global test_update_op

    #入力データを定義
    x =  tf.compat.v1.placeholder(tf.float32, [None, NUM_HAI])

    #重み
    w = []
    #バイアス
    b = []
    #隠れ層
    h = []

    #1層目の隠れ層
    w.append(tf.Variable(tf.compat.v1.random.truncated_normal([NUM_HAI, H_SIZE], stddev=0.1)))
    b.append(tf.Variable(tf.zeros([H_SIZE])))
    h.append(tf.nn.sigmoid(tf.matmul(x, w[0]) + b[0]))
    #h.append(tf.nn.relu(tf.matmul(x, w[0]) + b[0]))

    #2層目以降の隠れ層
    for i in range(1, NUM_HIDDEN_LAYERS):
        w.append(tf.Variable(tf.compat.v1.random.truncated_normal([H_SIZE, H_SIZE], stddev=0.1)))
        b.append(tf.Variable(tf.zeros([H_SIZE])))
        h.append(tf.nn.sigmoid(tf.matmul(h[i-1], w[i]) + b[i]))
        #h.append(tf.nn.relu(tf.matmul(h[i-1], w[i]) + b[i]))

    #出力層
    w.append(tf.Variable(tf.compat.v1.random.truncated_normal([H_SIZE, NUM_HAI], stddev=0.1)))
    b.append(tf.Variable(tf.zeros([NUM_HAI])))
    out = tf.nn.softmax(tf.matmul(h[NUM_HIDDEN_LAYERS-1], w[NUM_HIDDEN_LAYERS]) + b[NUM_HIDDEN_LAYERS])

    #正解データの型を定義
    y = tf.compat.v1.placeholder(tf.float32, [None, NUM_HAI])
    #誤差関数（クロスエントロピー）
    loss = tf.reduce_mean(-tf.reduce_sum(y * tf.math.log(out + 1e-5), axis=[1]))

    #訓練
    train_step = tf.compat.v1.train.GradientDescentOptimizer(0.01).minimize(loss)

    #評価
    #訓練データでの精度
    train_labels = tf.compat.v1.placeholder(tf.float32, [None, NUM_HAI])
    train_predictions = tf.compat.v1.placeholder(tf.float32, [None, NUM_HAI])
    train_accuracy, train_update_op = tf.compat.v1.metrics.accuracy(tf.argmax(train_labels, 1), tf.argmax(train_predictions, 1))
    #テストデータでの精度
    test_labels = tf.compat.v1.placeholder(tf.float32, [None, NUM_HAI])
    test_predictions = tf.compat.v1.placeholder(tf.float32, [None, NUM_HAI])
    test_accuracy, test_update_op = tf.compat.v1.metrics.accuracy(tf.argmax(test_labels, 1), tf.argmax(test_predictions, 1))


def train_ai(filename, num_of_epochs):
    mjl.load_dahai_data(filename)
    num_of_train_batches = mjl.get_num_of_train_batches()
    num_of_test_batches = mjl.get_num_of_test_batches()
    start_time = time.time()
    for i in range(num_of_epochs):
        for n in range(num_of_train_batches):
            #訓練
            batch_train_tehai = mjl.get_batch_train_tehai(n)
            batch_train_dahai = mjl.get_batch_train_dahai(n)
            sess.run(train_step, feed_dict={x:batch_train_tehai, y:batch_train_dahai})

        for n in range(num_of_train_batches):
            #訓練データに対する精度
            batch_train_tehai = mjl.get_batch_train_tehai(n)
            batch_train_dahai = mjl.get_batch_train_dahai(n)
            ai_outs = sess.run(out, feed_dict={x:batch_train_tehai})
            sess.run(train_update_op, feed_dict={train_labels:batch_train_dahai, train_predictions:ai_outs})

        for n in range(num_of_test_batches):
            #テストデータに対する精度
            batch_test_tehai = mjl.get_batch_test_tehai(n)
            batch_test_dahai = mjl.get_batch_test_dahai(n)
            ai_outs = sess.run(out, feed_dict={x:batch_test_tehai})
            sess.run(test_update_op, feed_dict={test_labels:batch_test_dahai, test_predictions:ai_outs})

        acc_val = sess.run(train_accuracy)
        print("Epoch {0}: train accuracy = {1}".format(i+1, acc_val))
        acc_val = sess.run(test_accuracy)
        print("Epoch {0}: test accuracy = {1}".format(i+1, acc_val))

    end_time = time.time()
    print("学習にかかった時間:{}秒".format(end_time-start_time))

def run_ai():
    test_count = 500
    agari_count = 0
    tenpai_count = 0
    total_point = 0
    tenpai_kuzusi_count = 0
    yaku_count = {}
    for _ in range(test_count):
        result, yaku_strings, tenpai_kuzusi = test_ai()
        if result > 0:
            agari_count += 1
            total_point += result
            for yaku in yaku_strings:
                if yaku in yaku_count:
                    yaku_count[yaku] += 1
                else:
                    yaku_count[yaku] = 1
        elif result == 0:
            tenpai_count += 1
        elif result == -1:
            if tenpai_kuzusi == True:
                tenpai_kuzusi_count += 1
        
    print("和了:" + str(agari_count))
    if agari_count > 0:
        print("和了時の平均点数:" + str(total_point / agari_count))
        print("役:" + str(yaku_count))        
    print("流局時聴牌:" + str(tenpai_count))
    print("聴牌崩し:" + str(tenpai_kuzusi_count))

def test_ai():
    mjc.init_yama()
    tehai = mjc.get_haipai()
    tsumo = -1
    tsumo_count = 0
    tenpai = False

    print("--------麻雀AIのテスト--------")
    while tsumo_count < 18:
        tstr = mjc.get_string_from_tehai(tehai)
        print("手牌:" + tstr)

        tsumo = mjc.get_tsumo()
        if tsumo == -1:
            break
        tsumo_count += 1
        print("自摸:" + mjc.get_hai_string(tsumo))
        tehai[tsumo] += 1
        if mjc.is_agari(tehai):
            print("和了")
            tehai[tsumo] -= 1
            info = mjc.AgariInfo(tehai, tsumo)
            print(info.get_yaku_strings())
            return (info.get_point(), info.get_yaku_strings(), False)
        
        ai_outs = sess.run(out, feed_dict={x:[tehai]})
        dahai = get_ai_dahai(tehai, ai_outs[0])
        print("打:" + mjc.get_hai_string(dahai))
        tehai[dahai] -= 1
        #一度でも聴牌したらフラグを立てる
        if tenpai == False and mjc.is_tenpai(tehai):
            tenpai = True

    if mjc.is_tenpai(tehai):
        print("流局(聴牌)")
        return (0, [], False)
    else:
        if tenpai == True:
            #聴牌を崩した
            print("流局(聴牌崩し)")
            return (-1, [], True)
        else:
            print("流局")
            return (-1, [], False)
			
def one_ai(one):
    tehai = mjc.get_tehai_from_string(one)
    if mjc.is_agari(tehai):
        print("和了")
        return -1
    ai_outs = sess.run(out, feed_dict={x:[tehai]})
    dahai = get_ai_dahai(tehai, ai_outs[0])
    print("--------麻雀AIのテスト--------")
    print("手牌:" + one)
    print("打:" + mjc.get_hai_string(dahai))
    return dahai

    
def get_ai_dahai(ai_in, ai_out):
    eval_idx = 0
    eval_max = 0
    while True:
        for i in range(len(ai_out)):
            if eval_max < ai_out[i]:
                eval_max = ai_out[i]
                eval_idx = i
        if ai_in[eval_idx] > 0:
            return eval_idx
        else:
            ai_out[eval_idx] = 0
            eval_max = 0


#自定义处理程序，用于处理HTTP请求  
class TestHTTPHandler(BaseHTTPRequestHandler):
    #处理GET请求  
    def do_GET(self):
        #获取URL
        print('URL='+self.path)
        data=self.path[1:len(self.path)].rstrip()
        data = urllib.parse.unquote(data)
        #页面输出模板字符串  
        templateStr = '''
        <html>   
        <head>   
        <title>QR Link Generator</title>   
        </head>   
        <body>   
        hello Python!
        </body>   
        </html>
        '''
        if data != "favicon.ico":
            templateStr = str(one_ai(data))
        self.protocal_version = 'HTTP/1.1'  #设置协议版本  
        self.send_response(200) #设置响应状态码  
        self.send_header("Welcome", "Contect")  #设置响应头  
        self.end_headers()
        self.wfile.write(templateStr.encode())   #输出响应内容  

        #启动服务函数  
def start_server(port):
        http_server = HTTPServer(('', int(port)), TestHTTPHandler)
        http_server.serve_forever() #设置一直监听并接收请求  

if __name__ == "__main__":
    #for windows
    sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8')
    arg_parser = argparse.ArgumentParser(prog = "mahjong_ai", add_help = True)
    arg_parser.add_argument("-r", "--run", help = "run AI", action = "store_true", default=False)
    arg_parser.add_argument("-t", "--train", help = "train AI", action = "store_true", default=True)
    arg_parser.add_argument("-e", "--epochs", help = "number of epochs", type=int, default=10)
    arg_parser.add_argument("-s", "--save", help="save model after training", action="store_true", default=False)
    arg_parser.add_argument("-m", "--model", help = "specify model", default=r"C:\Users\Dr.Red\Desktop\ZJU\Mahjong_python\ckpt\my_model")
    arg_parser.add_argument("-d", "--datafile", help="specify train data file", default = "dahai_data.txt")
    arg_parser.add_argument("-g", "--get", action="store_true", default=True)
    arg_parser.add_argument("-o", "--one", help="specify train data file", default = "1m3m4p6p8p9p9p3s3s4s7s7s9s中")
    args = arg_parser.parse_args()    

    do_train = False
    do_run = False

    # if args.get == True:
    #     do_run = False
    #     do_train = False
    # elif args.run == True and args.train == False:
    #     do_train = False
    # elif args.run == False and args.train == True:
    #     do_run = False
   
    make_model()
        # 创建一个 Saver 对象
    saver = tf.compat.v1.train.Saver()
        # 创建一个 Session 对象
    sess = tf.compat.v1.Session()
        # 初始化全局变量
    sess.run(tf.compat.v1.global_variables_initializer())
        # 初始化局部变量
    sess.run(tf.compat.v1.local_variables_initializer())
    

    if do_train:
            saver.restore(sess, args.model)
            train_ai(args.datafile, args.epochs)
            if args.save:
                saver.save(sess, args.model)
    else:
            saver.restore(sess, args.model)

    if do_run:
            run_ai()

    if args.get:           
            # start_server(8787)
            cap = cv2.VideoCapture(0)
            
            result_str  = ""
            robot_task_flag = False  # Indicates if it's the robot's turn to act
            arm_state_machine = process_ai.MahjongArmStateMachine_initial()

            #tile_position, destination = process_ai.process_ai_data_initial(0)


                # while True:
                #     #要加的内容包括：
                #         #手动提示有没有到他抓牌的回合
                #         # print("请确认是否到机械臂抓牌的回合，按q键确认", flush = True)
                #         #这里等待按键q的输入。如果按下q键，则开启周期
                #         #机械臂把牌放到正确牌堆里去
                #     #可能要涉及到多线程任务的处理，比如说start_server 之类的
                #     if not robot_task_flag:
                #         # User confirmation to switch to robot task
                #         user_input = input("请确认是否到机械臂抓牌的回合，按q键确认: ").strip()
                #     if user_input.lower() == 'q':
                #         print("确认开始机械臂任务...",flush = True)
                #         robot_task_flag = True
                #     else:
                #         print("输入错误，系统爆炸！",flush = True)
                #         exit(0)
                    
                #     if robot_task_flag:
                #         # Perform robot's task
                #         arm_state_machine.set_task(tile_position, destination)
                #         arm_state_machine.run_drawAtile()
                #         robot_task_flag = False  # Reset flag after task completion

                #     barcodes = None

                #     while not barcodes:
                #         ret, frame = cap.read()

                #         # Step 1: 灰度化
                #         gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

                #         # Step 2: 直方图均衡化
                #         equalized = cv2.equalizeHist(gray)

                #         # Step 3: 自适应阈值
                #         threshold = cv2.adaptiveThreshold(equalized, 255, cv2.ADAPTIVE_THRESH_GAUSSIAN_C,
                #                                         cv2.THRESH_BINARY, 11, 2)

                #         # 用 pyzbar 识别二维码
                #         barcodes = pyzbar.decode(threshold)  # 或 gray/equalized/edges，根据实际效果选择

                #         cv2.imshow("Barcode Scanner", threshold)

                #         if cv2.waitKey(1) & 0xFF == ord('q'):
                #             break
                #         # 定义result_str，初始化为长度为28的字符串，这里用'0'填充
            
                #         for barcode in barcodes:

                #             (x_1, y_1, w_1, h_1) = barcode.rect
                #             cv2.rectangle(frame, (x_1, y_1), (x_1 + w_1, y_1 + h_1), (0, 255, 0), 2)
        
                #             barcode_data = barcode.data.decode("utf-8")
                #             barcode_type = barcode.type
                
                #             #time.sleep(1) // 为了方便调试，注释掉了这一行
                #             # 确保barcode_data长度为2，如果不足2，则在前面补空格
                #             # barcode_data = barcode_data[:2].ljust(2, ' ') ;            
                #             # 将barcode_data写入到result_str的最后两个位置
                #             result_str = result_str + barcode_data
                        
                #             print("Result String:", result_str, flush = True)

                #             cv2.putText(frame, barcode_data, (x_1, y_1 - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 0), 2)
                #             # print("Barcode Type:{}, Barcode Data:{}".format(barcode_type, barcode_data))
                    
                #     arm_state_machine.run_drawAtile()

                #     # 使用正则表达式匹配数字+字母和汉字 
                #     pattern = r'\d+[A-Za-z]|[\u4e00-\u9fff]'
                #     # 初始化计数器
                #     count = 0

                #     # 循环匹配字符串中的项
                #     for match in re.finditer(pattern, result_str):
                #         # 每匹配到一项，计数器加1
                #         count += 1

                #     if count >= 13:
                #         break

                #     tile_position, destination = process_ai.process_ai_data_initial(count)

            tile_position,_ = process_ai.process_ai_data_discard(1,0)

            result_str = "1s2s6s1m3p2p1p2m4m9p北北9p"

            tehai =  mjc.get_outer_haipai(result_str)
            tsumo = -1
            tenpai = False
            check_if_agari = False

            count = 0

            while True:
                
                if not robot_task_flag:
                    # User confirmation to switch to robot task
                    user_input = input("请确认是否到机械臂抓牌的回合，按q键确认: ").strip()
                if user_input.lower() == 'q':
                    print("确认开始机械臂任务...",flush = True)
                    robot_task_flag = True
                else:
                    print("输入错误，系统爆炸！",flush = True)
                    exit(0)
                
                if robot_task_flag:
                    # Perform robot's task
                    arm_state_machine.set_task_discard(tile_position)
                    arm_state_machine.run_playAtile()
                    robot_task_flag = False  # Reset flag after task completion

                barcodes = None
                
                while not barcodes:
                    ret, frame = cap.read()
                    
                    # Step 1: 灰度化
                    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

                    # Step 2: 直方图均衡化
                    equalized = cv2.equalizeHist(gray)

                    # Step 3: 自适应阈值
                    threshold = cv2.adaptiveThreshold(equalized, 255, cv2.ADAPTIVE_THRESH_GAUSSIAN_C,
                                                    cv2.THRESH_BINARY, 11, 2)

                    # Step 4: 边缘检测（可选）
                    edges = cv2.Canny(threshold, 100, 200)

                    # 用 pyzbar 识别二维码
                    barcodes = pyzbar.decode(threshold)  # 或 gray/equalized/edges，根据实际效果选择

                    cv2.imshow("Barcode Scanner", frame)
                
                    if cv2.waitKey(1) & 0xFF == ord('q'):
                        break

                    # 定义result_str，初始化为长度为28的字符串，这里用'0'填充
        
                    for barcode in barcodes:

                        (x_1, y_1, w_1, h_1) = barcode.rect
                        cv2.rectangle(frame, (x_1, y_1), (x_1 + w_1, y_1 + h_1), (0, 255, 0), 2)
    
                        barcode_data = barcode.data.decode("utf-8")
                        barcode_type = barcode.type
            
                        time.sleep(1)
                        # 确保barcode_data长度为2，如果不足2，则在前面补空格
                        # barcode_data = barcode_data[:2].ljust(2, ' ')

                        cv2.putText(frame, barcode_data, (x_1, y_1 - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 0), 2)
                        # print("Barcode Type:{}, Barcode Data:{}".format(barcode_type, barcode_data))
                        tstr = mjc.get_string_from_tehai(tehai) + barcode_data

                print("手牌:" + tstr , flush = True)
                
                #一度でも聴牌したらフラグを立てる
                if tenpai == False and mjc.is_tenpai(tehai):
                    tenpai = True
                    print("俺聴牌!", flush = True)
                
                tsumo = mjc.get_outer_tsumo(barcode_data)
                if tsumo != -1:
                    print("自摸:" + mjc.get_hai_string(tsumo))
                
                tehai[tsumo] += 1
                if mjc.is_agari(tehai):
                    print("和了")
                    tehai[tsumo] -= 1
                    info = mjc.AgariInfo(tehai, tsumo)
                    print(info.get_yaku_strings())
                    check_if_agari = True
                    break
                    
                    # return (info.get_point(), info.get_yaku_strings(), False)

                ai_outs = sess.run(out, feed_dict={x:[tehai]})
                dahai = get_ai_dahai(tehai, ai_outs[0])
                print("打:" + mjc.get_hai_string(dahai))
                tehai[dahai] -= 1
                result_str, position = mjc.replace_and_find_position(mjc.get_hai_string(dahai),mjc.get_hai_string(tsumo), result_str)

                print("更新后的result_str:", result_str)
                print("替换的牌是第", position, "项")

                _,destination = process_ai.process_ai_data_discard(position ,count)

                arm_state_machine.set_task_discard1(destination)

                arm_state_machine.run_playAtile()

                count += 1

                tile_position, _= process_ai.process_ai_data_discard(position ,count)

                

                #如果胡牌则直接结束进程
                if check_if_agari :
                    break
            
            serial_comm.send_to_serial("$KMS:5,200,-10,1000!\r")
            serial_comm.send_to_serial("$KMS:10,150,30,1000!\r")
            serial_comm.send_to_serial("$KMS:10,150,-10,1000!\r")
            serial_comm.send_to_serial("$KMS:10,150,40,1000!\r")
            serial_comm.send_to_serial("$KMS:100,100,40,1000!\r")
            time.sleep(5)
            
            # 释放摄像头资源
            cap.release()

            # 关闭所有OpenCV窗口
            cv2.destroyAllWindows()


    

