from django.urls import path
from rest_framework.urlpatterns import format_suffix_patterns
from moistureduino import views

urlpatterns = [
    path('entries/', views.entry_list),
    path('entries/<int:pk>', views.entry_detail),
]

urlpatterns = format_suffix_patterns(urlpatterns)
