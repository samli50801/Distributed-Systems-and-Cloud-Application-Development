import boto3
import requests

inst_id = requests.get('http://169.254.169.254/latest/meta-data/instance-id')

# Create CloudWatch client
cloudwatch = boto3.client('cloudwatch', 'us-east-1')

# Create alarm
cloudwatch.put_metric_alarm(
    AlarmName='Web_Server_CPU_Utilization',
    ComparisonOperator='GreaterThanThreshold',
    EvaluationPeriods=1,
    MetricName='CPUUtilization',
    Namespace='AWS/EC2',
    Period=300,
    Statistic='Average',
    Threshold=70.0,
    ActionsEnabled=True,
    AlarmActions=[
      'arn:aws:autoscaling:us-east-1:666687831115:scalingPolicy:b1da9d3f-2ddf-4117-abf6-b7370eeb98f5:autoScalingGroupName/chord_ASG:policyName/policy-1'
    ],
    AlarmDescription='Alarm when server CPU exceeds 70%',
    Dimensions=[
        {
          'Name': 'AutoScalingGroupName',
          'Value': 'chord_ASG'
        },
    ]
)
