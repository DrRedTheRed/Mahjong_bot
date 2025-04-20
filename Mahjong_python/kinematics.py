import math
import time
import serial_comm

#define self.pi 3.1415926

class Kinematics():
    #设置四个关节的长度 mm
    #放大10倍
    L0 = 1000
    L1 = 1050
    L2 = 880
    L3 = 1550

    pi = 3.1415926

    time_temp = 1500

    servo_pwm0=0
    servo_pwm1=0
    servo_pwm2=0
    servo_pwm3=0
    servo_pwm4=0

    def kinematics_analysis(self, x:float, y:float, z:float, Alpha:float):
        '''
            x,y 为映射到平面的坐标
            z为距离地面的距离
            Alpha 为爪子和平面的夹角 -25~-65范围比较好
        '''
        #放大10倍
        x = x*10
        y = y*10
        z = z*10

        l0 = float(self.L0)
        l1 = float(self.L1)
        l2 = float(self.L2)
        l3 = float(self.L3)

        if x == 0:
            theta6 = 0.0
        else:
            theta6 = math.atan(x/y)*180.0/self.pi


        y = math.sqrt(x*x + y*y)
        y = y-l3 * math.cos(Alpha*self.pi/180.0)
        z = z-l0-l3*math.sin(Alpha*self.pi/180.0)
        if z < -l0:
            return 1
        if math.sqrt(y*y + z*z) > (l1+l2):
            return 2

        ccc = math.acos(y / math.sqrt(y * y + z * z))
        bbb = (y*y+z*z+l1*l1-l2*l2)/(2*l1*math.sqrt(y*y+z*z))
        if bbb > 1 or bbb < -1:
            return 5
        if z < 0:
            zf_flag = -1
        else:
            zf_flag = 1

        theta5 = ccc * zf_flag + math.acos(bbb)
        theta5 = theta5 * 180.0 / self.pi
        if theta5 > 180.0 or theta5 < 0.0:
            return 6

        aaa = -(y*y+z*z-l1*l1-l2*l2)/(2*l1*l2)
        if aaa > 1 or aaa < -1:
            return 3

        theta4 = math.acos(aaa)
        theta4 = 180.0 - theta4 * 180.0 / self.pi
        if theta4 > 135.0 or theta4 < -135.0:
            return 4

        theta3 = Alpha - theta5 + theta4
        if theta3 > 90.0 or theta3 < -90.0:
            return 7
        
        if theta6 < 0:
            theta2 = 90 + theta6
        else:
            theta2 = -90 + theta6 

        servo_angle0 = theta6
        servo_angle1 = theta5-90
        servo_angle2 = theta4
        servo_angle3 = theta3
        servo_angle4 = theta2

        self.servo_pwm0 = int(1500-2000.0 * servo_angle0 / 270.0)
        self.servo_pwm1 = int(1500+2000.0 * servo_angle1 / 270.0)
        self.servo_pwm2 = int(1500+2000.0 * servo_angle2 / 270.0)
        self.servo_pwm3 = int(1500-2000.0 * servo_angle3 / 270.0)
        self.servo_pwm4 = int(1500+2000.0 * servo_angle4 / 270.0)

        return ("{{#000P{0:0>4d}T{4:0>4d}!#001P{1:0>4d}T{4:0>4d}!#002P{2:0>4d}T{4:0>4d}!#003P{3:0>4d}T{4:0>4d}!}}".format(self.servo_pwm0,self.servo_pwm1,self.servo_pwm2,self.servo_pwm3,1000))

        # 机械臂逆运动
    def kinematics_move(self,x:float,y:float,z:float,time1:int)->int:

        if y < 0:
            return
        # 寻找最佳角度
        flag = 0
        cnt = 0
        for i in range(0, -136, -1):
            # print(isinstance(self.kinematics_analysis(x, y, z, i),str),i,self.kinematics_analysis(x, y, z, i))
            if  isinstance(self.kinematics_analysis(x, y, z, i),str):
                if i < cnt:
                    cnt = i
                flag = 1

        # 用3号舵机与水平最大的夹角作为最佳值
        if flag:
            self.kinematics_analysis(x, y, z, cnt)
            #根据机械臂ID号输出指令
            arm_str = ("{{#000P{0:0>4d}T{5:0>4d}!#001P{1:0>4d}T{5:0>4d}!#002P{2:0>4d}T{5:0>4d}!#003P{3:0>4d}T{5:0>4d}!#004P{4:0>4d}T{5:0>4d}!}}".
                       format(self.servo_pwm0,self.servo_pwm1,self.servo_pwm2,self.servo_pwm3,self.servo_pwm4,time1))

            serial_comm.send_to_serial(arm_str)



def trajectory_planning(start_point: tuple, end_point: tuple, trajectory_type: str):
    '''
    start_point: (x0, y0, z0)
    end_point: (x1, y1, z1)
    trajectory_type: 'linear' or 'parabola'
    '''
    x0, y0, z0 = start_point
    x1, y1, z1 = end_point

    points = []

    if trajectory_type == 'linear':
        steps = 3
        for i in range(steps + 1):
            t = i / steps
            x = x0 + t * (x1 - x0)
            y = y0 + t * (y1 - y0)
            z = z0 + t * (z1 - z0)
            points.append((x, y, z))

    elif trajectory_type == 'parabola':
        steps = 100
        for i in range(steps + 1):
            t = i / steps
            x = x0 + t * (x1 - x0)
            y = y0 + t * (y1 - y0)
            zMid = (z0 + z1) / 2 + 100
            z = (1 - t) * z0 + t * z1 + (4 * t * (1 - t) * (zMid - min(z0, z1)))
            points.append((x, y, z))

    return points

def move_as_tajectory(start_point, end_point, trajectory_type):
    points = trajectory_planning(start_point, end_point, trajectory_type)

    print(f"Moving from {start_point} to {end_point})",flush=True)

    for point in points:
        x, y, z = point
        arm = Kinematics()
        arm.kinematics_move(x, y, z, 1000)

