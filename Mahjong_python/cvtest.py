import cv2
from pyzbar import pyzbar

cap = cv2.VideoCapture(0)
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

    cv2.imshow("Barcode Scanner", threshold)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break
    # 定义result_str，初始化为长度为28的字符串，这里用'0'填充

    for barcode in barcodes:

        (x_1, y_1, w_1, h_1) = barcode.rect
        cv2.rectangle(frame, (x_1, y_1), (x_1 + w_1, y_1 + h_1), (0, 255, 0), 2)

        barcode_data = barcode.data.decode("utf-8")
        barcode_type = barcode.type

        #time.sleep(1) // 为了方便调试，注释掉了这一行
        # 确保barcode_data长度为2，如果不足2，则在前面补空格
        # barcode_data = barcode_data[:2].ljust(2, ' ')
        print("Barcode Data:", barcode_data, flush = True)

        cv2.putText(frame, barcode_data, (x_1, y_1 - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 0), 2)
        # print("Barcode Type:{}, Barcode Data:{}".format(barcode_type, barcode_data))